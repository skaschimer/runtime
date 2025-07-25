// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using Microsoft.DotNet.Cli.Build.Framework;
using Microsoft.NET.HostModel;
using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;

namespace Microsoft.DotNet.CoreSetup.Test.HostActivation
{
    public class RegisteredInstallLocationOverride : IDisposable
    {
        public string PathValueOverride { get; }

        // Windows only
        private readonly RegistryKey parentKey;
        private readonly RegistryKey key;
        private readonly string keyName;

        private IDisposable _testOnlyProductBehavior;

        public RegisteredInstallLocationOverride(string productBinaryPath)
        {
            _testOnlyProductBehavior = TestOnlyProductBehavior.Enable(productBinaryPath);

            if (OperatingSystem.IsWindows())
            {
                // To test registered installs, we need a registry key which is:
                // - writable without admin access - so that the tests don't require admin to run
                // - redirected in WOW64 - so that there are both 32-bit and 64-bit versions of the key
                //   This is because the product stores the info in the 32-bit hive only and even 64-bit
                //   product must look into the 32-bit hive.
                //   Without the redirection we would not be able to test that the product always looks
                //   into 32-bit only.
                // Per this page https://learn.microsoft.com/windows/desktop/WinProg64/shared-registry-keys
                // a user writable redirected key is for example HKCU\Software\Classes\Interface
                // so we're going to use that one - it's not super clean as the key stores COM interfaces,
                // but we should not corrupt anything by adding a special subkey even if it's left behind.
                //
                // Note: If you want to inspect the values written by the test and/or modify them manually
                //   you have to navigate to HKCU\Software\Classes\Wow6432Node\Interface on a 64-bit OS.
                using (RegistryKey hkcu = RegistryKey.OpenBaseKey(RegistryHive.CurrentUser, RegistryView.Registry32))
                {
                    parentKey = hkcu.CreateSubKey(@"Software\Classes\Interface");
                    keyName = "_DOTNET_Test" + Process.GetCurrentProcess().Id.ToString();
                    key = parentKey.CreateSubKey(keyName);
                    PathValueOverride = key.Name;
                }
            }
            else
            {
                // On Linux/macOS the install location is registered in a file which is normally
                // located in /etc/dotnet/install_location
                // So we need to redirect it to a different place here.
                string directory = Path.Combine(TestContext.TestArtifactsPath, "installLocationOverride" + Process.GetCurrentProcess().Id.ToString());
                if (Directory.Exists(directory))
                    Directory.Delete(directory, true);
                Directory.CreateDirectory(directory);
                PathValueOverride = directory;
            }
        }

        public void SetInstallLocation(params (string Architecture, string Path)[] locations)
        {
            Debug.Assert(locations.Length >= 1);
            if (OperatingSystem.IsWindows())
            {
                foreach (var location in locations)
                {
                    using (RegistryKey dotnetLocationKey = key.CreateSubKey($@"Setup\InstalledVersions\{location.Architecture}"))
                    {
                        dotnetLocationKey.SetValue("InstallLocation", location.Path);
                    }
                }
            }
            else
            {
                foreach (var location in locations)
                {
                    string installLocationFileName = "install_location" +
                        (!string.IsNullOrWhiteSpace(location.Architecture) ? ("_" + location.Architecture) : string.Empty);
                    File.WriteAllText(Path.Combine(PathValueOverride, installLocationFileName), location.Path);
                }
            }
        }

        public void Dispose()
        {
            if (OperatingSystem.IsWindows())
            {
                parentKey.DeleteSubKeyTree(keyName, throwOnMissingSubKey: false);
                key.Dispose();
                parentKey.Dispose();
            }
            else
            {
                RetryUtil.RetryOnIOError(() =>
                {
                    if (File.Exists(PathValueOverride))
                    {
                        File.Delete(PathValueOverride);
                    }
                });
            }

            if (_testOnlyProductBehavior != null)
            {
                _testOnlyProductBehavior.Dispose();
            }
        }
    }

    public static class RegisteredInstallLocationExtensions
    {
        public static Command ApplyRegisteredInstallLocationOverride(
            this Command command,
            RegisteredInstallLocationOverride registeredInstallLocationOverride)
        {
            if (registeredInstallLocationOverride == null)
            {
                return command;
            }

            if (OperatingSystem.IsWindows())
            {
                return command.EnvironmentVariable(
                    Constants.TestOnlyEnvironmentVariables.RegistryPath,
                    registeredInstallLocationOverride.PathValueOverride);
            }
            else
            {
                return command.EnvironmentVariable(
                    Constants.TestOnlyEnvironmentVariables.InstallLocationPath,
                    registeredInstallLocationOverride.PathValueOverride);
            }
        }
    }
}
