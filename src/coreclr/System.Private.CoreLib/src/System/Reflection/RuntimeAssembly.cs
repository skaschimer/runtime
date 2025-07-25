// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration.Assemblies;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Runtime.Loader;
using System.Runtime.Serialization;
using System.Security;
using System.Threading;
using CultureInfo = System.Globalization.CultureInfo;
using StackCrawlMark = System.Threading.StackCrawlMark;

namespace System.Reflection
{
    internal sealed partial class RuntimeAssembly : Assembly
    {
        internal RuntimeAssembly() { throw new NotSupportedException(); }

        #region private data members
#pragma warning disable 67 // events are declared but not used
        private event ModuleResolveEventHandler? _ModuleResolve;
#pragma warning restore 67
        private string? m_fullname;
        private object? m_syncRoot;   // Used to keep collectible types alive and as the syncroot for reflection.emit
#pragma warning disable 169
        private IntPtr m_assembly;    // slack for ptr datum on unmanaged side
#pragma warning restore 169

        #endregion

        internal IntPtr GetUnderlyingNativeHandle() =>  m_assembly;

        private sealed class ManifestResourceStream : UnmanagedMemoryStream
        {
            // ensures the RuntimeAssembly is kept alive for as long as the stream lives
            private readonly RuntimeAssembly _manifestAssembly;

            internal unsafe ManifestResourceStream(RuntimeAssembly manifestAssembly, byte* pointer, long length, long capacity, FileAccess access) : base(pointer, length, capacity, access)
            {
                _manifestAssembly = manifestAssembly;
            }

            // override Read(Span<byte>) because the base UnmanagedMemoryStream doesn't optimize it for derived types
            public override int Read(Span<byte> buffer) => ReadCore(buffer);

            // NOTE: no reason to override Write(Span<byte>), since a ManifestResourceStream is read-only.
        }

        internal object SyncRoot =>
            m_syncRoot ?? Interlocked.CompareExchange(ref m_syncRoot, new object(), null) ?? m_syncRoot;

        public override event ModuleResolveEventHandler? ModuleResolve
        {
            add
            {
                _ModuleResolve += value;
            }
            remove
            {
                _ModuleResolve -= value;
            }
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetCodeBase")]
        [return: MarshalAs(UnmanagedType.Bool)]
        private static partial bool GetCodeBase(QCallAssembly assembly,
                                               StringHandleOnStack retString);

        internal string? GetCodeBase()
        {
            string? codeBase = null;
            RuntimeAssembly runtimeAssembly = this;
            if (GetCodeBase(new QCallAssembly(ref runtimeAssembly), new StringHandleOnStack(ref codeBase)))
            {
                return codeBase;
            }
            return null;
        }

        [Obsolete("Assembly.CodeBase and Assembly.EscapedCodeBase are only included for .NET Framework compatibility. Use Assembly.Location.", DiagnosticId = "SYSLIB0012", UrlFormat = "https://aka.ms/dotnet-warnings/{0}")]
        [RequiresAssemblyFiles(ThrowingMessageInRAF)]
        public override string? CodeBase
        {
            get
            {
                if (IsDynamic)
                {
                    throw new NotSupportedException(SR.NotSupported_DynamicAssembly);
                }

                string? codeBase = GetCodeBase() ?? throw new NotSupportedException(SR.NotSupported_CodeBase);
                if (codeBase.Length == 0)
                {
                    // For backward compatibility, return CoreLib codebase for assemblies loaded from memory.
#pragma warning disable SYSLIB0012
                    codeBase = typeof(object).Assembly.CodeBase;
#pragma warning restore SYSLIB0012
                }
                return codeBase;
            }
        }

        // If the assembly is copied before it is loaded, the codebase will be set to the
        // actual file loaded if copiedName is true. If it is false, then the original code base
        // is returned.
        public override AssemblyName GetName(bool copiedName)
        {
            var an = new AssemblyName();
            an.Name = GetSimpleName();
            an.Version = GetVersion();
            an.CultureInfo = GetLocale();

            an.SetPublicKey(GetPublicKey());

            an.RawFlags = GetFlags() | AssemblyNameFlags.PublicKey;

#pragma warning disable IL3000, SYSLIB0044 // System.Reflection.AssemblyName.CodeBase' always returns an empty string for assemblies embedded in a single-file app.
                                           // AssemblyName.CodeBase and AssemblyName.EscapedCodeBase are obsolete. Using them for loading an assembly is not supported.
            an.CodeBase = GetCodeBase();
#pragma warning restore IL3000, SYSLIB0044

#pragma warning disable SYSLIB0037 // AssemblyName.HashAlgorithm is obsolete
            an.HashAlgorithm = GetHashAlgorithm();
#pragma warning restore SYSLIB0037

            Module manifestModule = ManifestModule;
            if (manifestModule.MDStreamVersion > 0x10000)
            {
                manifestModule.GetPEKind(out PortableExecutableKinds pek, out ImageFileMachine ifm);
                an.SetProcArchIndex(pek, ifm);
            }
            return an;
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetFullName")]
        private static partial void GetFullName(QCallAssembly assembly, StringHandleOnStack retString);

        public override string? FullName
        {
            get
            {
                // If called by Object.ToString(), return val may be NULL.
                if (m_fullname == null)
                {
                    string? s = null;
                    RuntimeAssembly runtimeAssembly = this;
                    GetFullName(new QCallAssembly(ref runtimeAssembly), new StringHandleOnStack(ref s));
                    Interlocked.CompareExchange(ref m_fullname, s, null);
                }

                return m_fullname;
            }
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetEntryPoint")]
        private static partial void GetEntryPoint(QCallAssembly assembly, ObjectHandleOnStack retMethod);

        public override MethodInfo? EntryPoint
        {
            get
            {
                IRuntimeMethodInfo? methodHandle = null;
                RuntimeAssembly runtimeAssembly = this;
                GetEntryPoint(new QCallAssembly(ref runtimeAssembly), ObjectHandleOnStack.Create(ref methodHandle));

                if (methodHandle == null)
                    return null;

                return (MethodInfo?)RuntimeType.GetMethodBase(methodHandle);
            }
        }

        // For case-sensitive lookups, marshal the strings directly to Utf8 to avoid unnecessary string copies.
        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetTypeCore", StringMarshalling = StringMarshalling.Utf8)]
        private static partial void GetTypeCore(QCallAssembly assembly,
                                            string typeName,
                                            ReadOnlySpan<string> nestedTypeNames,
                                            int nestedTypeNamesLength,
                                            ObjectHandleOnStack retType);

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetTypeCoreIgnoreCase", StringMarshalling = StringMarshalling.Utf16)]
        private static partial void GetTypeCoreIgnoreCase(QCallAssembly assembly,
                                            string typeName,
                                            ReadOnlySpan<string> nestedTypeNames,
                                            int nestedTypeNamesLength,
                                            ObjectHandleOnStack retType);

        internal Type? GetTypeCore(string typeName, ReadOnlySpan<string> nestedTypeNames, bool throwOnFileNotFound, bool ignoreCase)
        {
            RuntimeAssembly runtimeAssembly = this;
            Type? type = null;

            try
            {
                if (ignoreCase)
                {
                    GetTypeCoreIgnoreCase(new QCallAssembly(ref runtimeAssembly),
                        typeName,
                        nestedTypeNames,
                        nestedTypeNames.Length,
                        ObjectHandleOnStack.Create(ref type));
                }
                else
                {
                    GetTypeCore(new QCallAssembly(ref runtimeAssembly),
                        typeName,
                        nestedTypeNames,
                        nestedTypeNames.Length,
                        ObjectHandleOnStack.Create(ref type));
                }
            }
            catch (FileNotFoundException) when (!throwOnFileNotFound)
            {
                return null;
            }

            return type;
        }

        [RequiresUnreferencedCode("Types might be removed by trimming. If the type name is a string literal, consider using Type.GetType instead.")]
        public override Type? GetType(
            string name, // throw on null strings regardless of the value of "throwOnError"
            bool throwOnError, bool ignoreCase)
        {
            ArgumentException.ThrowIfNullOrEmpty(name);

            return TypeNameResolver.GetType(name, topLevelAssembly: this,
                throwOnError: throwOnError, ignoreCase: ignoreCase);
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetExportedTypes")]
        private static partial void GetExportedTypes(QCallAssembly assembly, ObjectHandleOnStack retTypes);

        [RequiresUnreferencedCode("Types might be removed")]
        public override Type[] GetExportedTypes()
        {
            Type[]? types = null;
            RuntimeAssembly runtimeAssembly = this;
            GetExportedTypes(new QCallAssembly(ref runtimeAssembly), ObjectHandleOnStack.Create(ref types));
            return types!;
        }

        public override IEnumerable<TypeInfo> DefinedTypes
        {
            [RequiresUnreferencedCode("Types might be removed")]
            get => GetManifestModule().GetDefinedTypes();
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetIsCollectible")]
        internal static partial Interop.BOOL GetIsCollectible(QCallAssembly assembly);

        public override bool IsCollectible
        {
            get
            {
                RuntimeAssembly runtimeAssembly = this;
                return GetIsCollectible(new QCallAssembly(ref runtimeAssembly)) != Interop.BOOL.FALSE;
            }
        }

        // GetResource will return a pointer to the resources in memory.
        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetResource", StringMarshalling = StringMarshalling.Utf16)]
        private static unsafe partial byte* GetResource(QCallAssembly assembly,
                                                       string resourceName,
                                                       out uint length);

        // Load a resource based on the NameSpace of the type.
        public override Stream? GetManifestResourceStream(Type type, string name)
        {
            if (name == null)
                ArgumentNullException.ThrowIfNull(type);

            string? nameSpace = type?.Namespace;

            char c = Type.Delimiter;
            string resourceName = nameSpace != null && name != null ?
                string.Concat(nameSpace, new ReadOnlySpan<char>(in c), name) :
                string.Concat(nameSpace, name);

            return GetManifestResourceStream(resourceName);
        }

        public override unsafe Stream? GetManifestResourceStream(string name)
        {
            RuntimeAssembly runtimeAssembly = this;
            byte* pbInMemoryResource = GetResource(new QCallAssembly(ref runtimeAssembly), name, out uint length);

            if (pbInMemoryResource != null)
            {
                return new ManifestResourceStream(this, pbInMemoryResource, length, length, FileAccess.Read);
            }

            return null;
        }

        // ISerializable implementation
        [Obsolete(Obsoletions.LegacyFormatterImplMessage, DiagnosticId = Obsoletions.LegacyFormatterImplDiagId, UrlFormat = Obsoletions.SharedUrlFormat)]
        [EditorBrowsable(EditorBrowsableState.Never)]
        public override void GetObjectData(SerializationInfo info, StreamingContext context)
        {
            throw new PlatformNotSupportedException();
        }

        public override Module ManifestModule =>
            // We don't need to return the "external" ModuleBuilder because
            // it is meant to be read-only
            GetManifestModule();

        public override object[] GetCustomAttributes(bool inherit)
        {
            return CustomAttribute.GetCustomAttributes(this, (typeof(object) as RuntimeType)!);
        }

        public override object[] GetCustomAttributes(Type attributeType, bool inherit)
        {
            ArgumentNullException.ThrowIfNull(attributeType);

            if (attributeType.UnderlyingSystemType is not RuntimeType attributeRuntimeType)
                throw new ArgumentException(SR.Arg_MustBeType, nameof(attributeType));

            return CustomAttribute.GetCustomAttributes(this, attributeRuntimeType);
        }

        public override bool IsDefined(Type attributeType, bool inherit)
        {
            ArgumentNullException.ThrowIfNull(attributeType);

            if (attributeType.UnderlyingSystemType is not RuntimeType attributeRuntimeType)
                throw new ArgumentException(SR.Arg_MustBeType, nameof(attributeType));

            return CustomAttribute.IsDefined(this, attributeRuntimeType);
        }

        public override IList<CustomAttributeData> GetCustomAttributesData()
        {
            return RuntimeCustomAttributeData.GetCustomAttributesInternal(this);
        }

        internal static RuntimeAssembly InternalLoad(string assemblyName, ref StackCrawlMark stackMark, AssemblyLoadContext? assemblyLoadContext = null)
            => InternalLoad(new AssemblyName(assemblyName), ref stackMark, assemblyLoadContext);

        internal static unsafe RuntimeAssembly InternalLoad(AssemblyName assemblyName,
                                                            ref StackCrawlMark stackMark,
                                                            AssemblyLoadContext? assemblyLoadContext = null,
                                                            RuntimeAssembly? requestingAssembly = null,
                                                            bool throwOnFileNotFound = true)
        {
            RuntimeAssembly? retAssembly = null;

            AssemblyNameFlags flags = assemblyName.RawFlags;

            // Note that we prefer to take a public key token if present,
            // even if flags indicate a full public key
            byte[]? publicKeyOrToken;
            if ((publicKeyOrToken = assemblyName.RawPublicKeyToken) != null)
            {
                flags &= ~AssemblyNameFlags.PublicKey;
            }
            else if ((publicKeyOrToken = assemblyName.RawPublicKey) != null)
            {
                flags |= AssemblyNameFlags.PublicKey;
            }

            fixed (char* pName = assemblyName.Name)
            fixed (char* pCultureName = assemblyName.CultureName)
            fixed (byte* pPublicKeyOrToken = publicKeyOrToken)
            {
                NativeAssemblyNameParts nameParts = default;

                nameParts._flags = flags;
                nameParts._pName = pName;
                nameParts._pCultureName = pCultureName;

                nameParts._pPublicKeyOrToken = pPublicKeyOrToken;
                nameParts._cbPublicKeyOrToken = (publicKeyOrToken != null) ? publicKeyOrToken.Length : 0;

                nameParts.SetVersion(assemblyName.Version, defaultValue: ushort.MaxValue);

                InternalLoad(&nameParts,
                             ObjectHandleOnStack.Create(ref requestingAssembly),
                             new StackCrawlMarkHandle(ref stackMark),
                             throwOnFileNotFound,
                             ObjectHandleOnStack.Create(ref assemblyLoadContext),
                             ObjectHandleOnStack.Create(ref retAssembly));
            }

            return retAssembly!;
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_InternalLoad")]
        private static unsafe partial void InternalLoad(NativeAssemblyNameParts* pAssemblyNameParts,
                                                ObjectHandleOnStack requestingAssembly,
                                                StackCrawlMarkHandle stackMark,
                                                [MarshalAs(UnmanagedType.Bool)] bool throwOnFileNotFound,
                                                ObjectHandleOnStack assemblyLoadContext,
                                                ObjectHandleOnStack retAssembly);

        public override bool ReflectionOnly => false;

        // Returns the module in this assembly with name 'name'

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetModule", StringMarshalling = StringMarshalling.Utf16)]
        private static partial void GetModule(QCallAssembly assembly, string name, ObjectHandleOnStack retModule);

        public override Module? GetModule(string name)
        {
            Module? retModule = null;
            RuntimeAssembly runtimeAssembly = this;
            GetModule(new QCallAssembly(ref runtimeAssembly), name, ObjectHandleOnStack.Create(ref retModule));
            return retModule;
        }

        // Returns the file in the File table of the manifest that matches the
        // given name.  (Name should not include path.)
        [RequiresAssemblyFiles(ThrowingMessageInRAF)]
        public override FileStream? GetFile(string name)
        {
            if (Location.Length == 0)
            {
                // Throw if the assembly was loaded from memory, indicated by Location returning an empty string
                throw new FileNotFoundException(SR.IO_NoFileTableInInMemoryAssemblies);
            }

            RuntimeModule? m = (RuntimeModule?)GetModule(name);
            if (m == null)
                return null;

            return new FileStream(m.GetFullyQualifiedName(),
                                  FileMode.Open,
                                  FileAccess.Read, FileShare.Read, FileStream.DefaultBufferSize, false);
        }

        [RequiresAssemblyFiles(ThrowingMessageInRAF)]
        public override FileStream[] GetFiles(bool getResourceModules)
        {
            if (Location.Length == 0)
            {
                // Throw if the assembly was loaded from memory, indicated by Location returning an empty string
                throw new FileNotFoundException(SR.IO_NoFileTableInInMemoryAssemblies);
            }

            Module[] m = GetModules(getResourceModules);
            FileStream[] fs = new FileStream[m.Length];

            for (int i = 0; i < fs.Length; i++)
            {
                fs[i] = new FileStream(((RuntimeModule)m[i]).GetFullyQualifiedName(),
                                       FileMode.Open,
                                       FileAccess.Read, FileShare.Read, FileStream.DefaultBufferSize, false);
            }

            return fs;
        }

        // Returns the names of all the resources
        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetManifestResourceNames")]
        private static partial void GetManifestResourceNames(QCallAssembly assembly, ObjectHandleOnStack retResourceNames);

        // Returns the names of all the resources
        public override string[] GetManifestResourceNames()
        {
            string[]? resourceNames = null;
            RuntimeAssembly runtimeAssembly = this;
            GetManifestResourceNames(new QCallAssembly(ref runtimeAssembly), ObjectHandleOnStack.Create(ref resourceNames));
            return resourceNames!;
        }

        // Returns the names of all the resources
        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetReferencedAssemblies")]
        private static partial void GetReferencedAssemblies(QCallAssembly assembly, ObjectHandleOnStack retReferencedAssemblies);

        [RequiresUnreferencedCode("Assembly references might be removed")]
        public override AssemblyName[] GetReferencedAssemblies()
        {
            AssemblyName[]? referencedAssemblies = null;
            RuntimeAssembly runtimeAssembly = this;
            GetReferencedAssemblies(new QCallAssembly(ref runtimeAssembly), ObjectHandleOnStack.Create(ref referencedAssemblies));
            return referencedAssemblies!;
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetManifestResourceInfo", StringMarshalling = StringMarshalling.Utf16)]
        private static partial int GetManifestResourceInfo(QCallAssembly assembly,
                                                          string resourceName,
                                                          ObjectHandleOnStack assemblyRef,
                                                          StringHandleOnStack retFileName);

        public override ManifestResourceInfo? GetManifestResourceInfo(string resourceName)
        {
            RuntimeAssembly? retAssembly = null;
            string? fileName = null;
            RuntimeAssembly runtimeAssembly = this;
            int location = GetManifestResourceInfo(new QCallAssembly(ref runtimeAssembly), resourceName,
                                                   ObjectHandleOnStack.Create(ref retAssembly),
                                                   new StringHandleOnStack(ref fileName));

            if (location == -1)
                return null;

            return new ManifestResourceInfo(retAssembly!, fileName!,
                                                (ResourceLocation)location);
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetLocation")]
        private static partial void GetLocation(QCallAssembly assembly, StringHandleOnStack retString);

        public override string Location
        {
            get
            {
                string? location = null;

                RuntimeAssembly runtimeAssembly = this;
                GetLocation(new QCallAssembly(ref runtimeAssembly), new StringHandleOnStack(ref location));

                return location!;
            }
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetImageRuntimeVersion")]
        private static partial void GetImageRuntimeVersion(QCallAssembly assembly, StringHandleOnStack retString);

        public override string ImageRuntimeVersion
        {
            get
            {
                string? s = null;
                RuntimeAssembly runtimeAssembly = this;
                GetImageRuntimeVersion(new QCallAssembly(ref runtimeAssembly), new StringHandleOnStack(ref s));
                return s!;
            }
        }

        [Obsolete(Obsoletions.GlobalAssemblyCacheMessage, DiagnosticId = Obsoletions.GlobalAssemblyCacheDiagId, UrlFormat = Obsoletions.SharedUrlFormat)]
        public override bool GlobalAssemblyCache => false;

        public override long HostContext => 0;

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetVersion")]
        private static partial void GetVersion(QCallAssembly assembly,
                                              out int majVer,
                                              out int minVer,
                                              out int buildNum,
                                              out int revNum);

        private Version GetVersion()
        {
            RuntimeAssembly runtimeAssembly = this;
            GetVersion(new QCallAssembly(ref runtimeAssembly), out int majorVer, out int minorVer, out int build, out int revision);
            return new Version(majorVer, minorVer, build, revision);
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetLocale")]
        private static partial void GetLocale(QCallAssembly assembly, StringHandleOnStack retString);

        private CultureInfo GetLocale()
        {
            string? locale = null;

            RuntimeAssembly runtimeAssembly = this;
            GetLocale(new QCallAssembly(ref runtimeAssembly), new StringHandleOnStack(ref locale));

            if (locale == null)
                return CultureInfo.InvariantCulture;

            return CultureInfo.GetCultureInfo(locale);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool GetIsDynamic(IntPtr assembly);

        public override bool IsDynamic
        {
            get
            {
                bool isDynamic = GetIsDynamic(GetUnderlyingNativeHandle());
                GC.KeepAlive(this); // We directly pass the native handle above - make sure this object stays alive for the call
                return isDynamic;
            }
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetSimpleName")]
        private static partial void GetSimpleName(QCallAssembly assembly, StringHandleOnStack retSimpleName);

        internal string GetSimpleName()
        {
            RuntimeAssembly runtimeAssembly = this;
            string? name = null;
            GetSimpleName(new QCallAssembly(ref runtimeAssembly), new StringHandleOnStack(ref name));
            return name!;
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetHashAlgorithm")]
        private static partial AssemblyHashAlgorithm GetHashAlgorithm(QCallAssembly assembly);

        private AssemblyHashAlgorithm GetHashAlgorithm()
        {
            RuntimeAssembly runtimeAssembly = this;
            return GetHashAlgorithm(new QCallAssembly(ref runtimeAssembly));
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetFlags")]
        private static partial AssemblyNameFlags GetFlags(QCallAssembly assembly);

        private AssemblyNameFlags GetFlags()
        {
            RuntimeAssembly runtimeAssembly = this;
            return GetFlags(new QCallAssembly(ref runtimeAssembly));
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetPublicKey")]
        private static partial void GetPublicKey(QCallAssembly assembly, ObjectHandleOnStack retPublicKey);

        internal byte[]? GetPublicKey()
        {
            byte[]? publicKey = null;
            RuntimeAssembly runtimeAssembly = this;
            GetPublicKey(new QCallAssembly(ref runtimeAssembly), ObjectHandleOnStack.Create(ref publicKey));
            return publicKey;
        }

        public override Assembly GetSatelliteAssembly(CultureInfo culture)
        {
            return GetSatelliteAssembly(culture, null);
        }

        // Useful for binding to a very specific version of a satellite assembly
        public override Assembly GetSatelliteAssembly(CultureInfo culture, Version? version)
        {
            ArgumentNullException.ThrowIfNull(culture);

            return InternalGetSatelliteAssembly(this, culture, version, throwOnFileNotFound: true)!;
        }

        [DynamicSecurityMethod] // Methods containing StackCrawlMark local var has to be marked DynamicSecurityMethod
        internal static Assembly? InternalGetSatelliteAssembly(Assembly assembly,
                                                       CultureInfo culture,
                                                       Version? version,
                                                       bool throwOnFileNotFound)
        {
            var an = new AssemblyName();
            RuntimeAssembly runtimeAssembly = (RuntimeAssembly)assembly;
            an.SetPublicKey(runtimeAssembly.GetPublicKey());
            an.Flags = runtimeAssembly.GetFlags() | AssemblyNameFlags.PublicKey;
            an.Version = version ?? runtimeAssembly.GetVersion();
            an.CultureInfo = culture;
            an.Name = runtimeAssembly.GetSimpleName() + ".resources";

            // This stack crawl mark is never used because the requesting assembly is explicitly specified,
            // so the value could be anything.
            StackCrawlMark unused = default;
            RuntimeAssembly? retAssembly = InternalLoad(an, ref unused, requestingAssembly: runtimeAssembly, throwOnFileNotFound: throwOnFileNotFound);

            if (retAssembly == runtimeAssembly)
            {
                retAssembly = null;
            }

            if (retAssembly == null && throwOnFileNotFound)
            {
                throw new FileNotFoundException(SR.Format(culture, SR.IO_FileNotFound_FileName, an.Name));
            }

            return retAssembly;
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetModules")]
        private static partial void GetModules(QCallAssembly assembly,
                                              [MarshalAs(UnmanagedType.Bool)] bool loadIfNotFound,
                                              [MarshalAs(UnmanagedType.Bool)] bool getResourceModules,
                                              ObjectHandleOnStack retModuleHandles);

        private RuntimeModule[] GetModulesInternal(bool loadIfNotFound,
                                     bool getResourceModules)
        {
            RuntimeModule[]? modules = null;
            RuntimeAssembly runtimeAssembly = this;

            GetModules(new QCallAssembly(ref runtimeAssembly), loadIfNotFound, getResourceModules, ObjectHandleOnStack.Create(ref modules));
            return modules!;
        }

        public override Module[] GetModules(bool getResourceModules)
        {
            return GetModulesInternal(true, getResourceModules);
        }

        public override Module[] GetLoadedModules(bool getResourceModules)
        {
            return GetModulesInternal(false, getResourceModules);
        }

        private RuntimeModule GetManifestModule()
        {
            return GetManifestModule(this) ?? GetManifestModuleWorker(this);

            [MethodImpl(MethodImplOptions.NoInlining)]
            static RuntimeModule GetManifestModuleWorker(RuntimeAssembly assembly)
            {
                RuntimeModule? module = null;
                GetManifestModuleSlow(ObjectHandleOnStack.Create(ref assembly), ObjectHandleOnStack.Create(ref module));
                return module!;
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern RuntimeModule? GetManifestModule(RuntimeAssembly assembly);

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyHandle_GetManifestModuleSlow")]
        private static partial void GetManifestModuleSlow(ObjectHandleOnStack assembly, ObjectHandleOnStack module);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern int GetTokenInternal(RuntimeAssembly assembly);

        internal static int GetToken(RuntimeAssembly assembly)
        {
            int tokenMaybe = GetTokenInternal(assembly);
            // If the result is negative, it is an error code.
            if (tokenMaybe < 0)
                Marshal.ThrowExceptionForHR(tokenMaybe, new IntPtr(-1));
            return tokenMaybe;
        }

        [RequiresUnreferencedCode("Types might be removed")]
        public sealed override Type[] GetForwardedTypes()
        {
            List<Type> types = new List<Type>();
            List<Exception> exceptions = new List<Exception>();

            MetadataImport scope = GetManifestModule().MetadataImport;
            scope.Enum(MetadataTokenType.ExportedType, 0, out MetadataEnumResult enumResult);
            RuntimeAssembly runtimeAssembly = this;
            QCallAssembly pAssembly = new QCallAssembly(ref runtimeAssembly);
            for (int i = 0; i < enumResult.Length; i++)
            {
                MetadataToken mdtExternalType = enumResult[i];
                Type? type = null;
                Exception? exception = null;
                ObjectHandleOnStack pType = ObjectHandleOnStack.Create(ref type);
                try
                {
                    GetForwardedType(pAssembly, mdtExternalType, pType);
                    if (type == null)
                        continue;  // mdtExternalType was not a forwarder entry.
                }
                catch (Exception e)
                {
                    type = null;
                    exception = e;
                }

                Debug.Assert((type != null) != (exception != null)); // Exactly one of these must be non-null.

                if (type != null)
                {
                    types.Add(type);
                    AddPublicNestedTypes(type, types, exceptions);
                }
                else
                {
                    exceptions.Add(exception!);
                }
            }

            if (exceptions.Count != 0)
            {
                int numTypes = types.Count;
                int numExceptions = exceptions.Count;
                types.AddRange(new Type[numExceptions]); // add one null Type for each exception.
                exceptions.InsertRange(0, new Exception[numTypes]); // align the Exceptions with the null Types.
                throw new ReflectionTypeLoadException(types.ToArray(), exceptions.ToArray());
            }

            return types.ToArray();
        }

        [RequiresUnreferencedCode("Types might be removed because recursive nested types can't currently be annotated for dynamic access.")]
        private static void AddPublicNestedTypes(Type type, List<Type> types, List<Exception> exceptions)
        {
            Type[] nestedTypes;
            try
            {
                nestedTypes = type.GetNestedTypes(BindingFlags.Public);
            }
            catch (Exception e)
            {
                exceptions.Add(e);
                return;
            }
            foreach (Type nestedType in nestedTypes)
            {
                types.Add(nestedType);
                AddPublicNestedTypes(nestedType, types, exceptions);
            }
        }

        [LibraryImport(RuntimeHelpers.QCall, EntryPoint = "AssemblyNative_GetForwardedType")]
        private static partial void GetForwardedType(QCallAssembly assembly, MetadataToken mdtExternalType, ObjectHandleOnStack type);
    }
}
