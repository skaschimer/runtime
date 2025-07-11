// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

using System.Collections.Generic;
using System.Text;

namespace ILCompiler.Reflection.ReadyToRun
{
    /// <summary>
    /// based on <a href="https://github.com/dotnet/runtime/blob/main/src/coreclr/vm/nativeformatreader.h">NativeFormat::NativeParser</a>
    /// </summary>
    public struct NativeParser
    {
        // TODO (refactoring) - all these Native* class should be private
        /// <summary>
        /// The current index of the image byte array
        /// </summary>
        public uint Offset { get; set; }
        public byte LowHashcode { get; }

        NativeReader _imageReader;

        public NativeParser(NativeReader imageReader, uint offset, byte lowHashcode = 0)
        {
            Offset = offset;
            LowHashcode = lowHashcode;
            _imageReader = imageReader;
        }

        public bool IsNull()
        {
            return _imageReader == null;
        }

        public uint GetRelativeOffset()
        {
            uint pos = Offset;

            int delta = 0;
            Offset = _imageReader.DecodeSigned(Offset, ref delta);

            return pos + (uint)delta;
        }

        public NativeParser GetParserFromRelativeOffset()
        {
            byte lowHashcode = GetByte();
            return new NativeParser(_imageReader, GetRelativeOffset(), lowHashcode);
        }

        public byte GetByte()
        {
            int off = (int)Offset;
            byte val = _imageReader.ReadByte(ref off);
            Offset += 1;
            return val;
        }

        public uint GetCompressedData()
        {
            int off = (int)Offset;
            uint val = _imageReader.ReadCompressedData(ref off);
            Offset = (uint)off;
            return val;
        }

        public uint GetUnsigned()
        {
            uint value = 0;
            Offset = _imageReader.DecodeUnsigned(Offset, ref value);
            return value;
        }

        public int GetSigned()
        {
            int value = 0;
            Offset = _imageReader.DecodeSigned(Offset, ref value);
            return value;
        }
    }

    /// <summary>
    /// based on <a href="https://github.com/dotnet/runtime/blob/main/src/coreclr/vm/nativeformatreader.h">NativeFormat::NativeHashtable</a>
    /// </summary>
    public struct NativeHashtable
    {
        // TODO (refactoring) - all these Native* class should be private
        private NativeReader _imageReader;
        private uint _baseOffset;
        private uint _bucketMask;
        private byte _entryIndexSize;
        private uint _endOffset;

        public NativeHashtable(NativeReader imageReader, NativeParser parser, uint endOffset)
        {
            uint header = parser.GetByte();
            _baseOffset = parser.Offset;
            _imageReader = imageReader;

            int numberOfBucketsShift = (int)(header >> 2);
            if (numberOfBucketsShift > 31)
                throw new System.BadImageFormatException();
            _bucketMask = (uint)((1 << numberOfBucketsShift) - 1);

            byte entryIndexSize = (byte)(header & 3);
            if (entryIndexSize > 2)
                throw new System.BadImageFormatException();
            _entryIndexSize = entryIndexSize;

            _endOffset = endOffset;
        }

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();

            SortedDictionary<uint, byte> entries = new SortedDictionary<uint, byte>();
            AllEntriesEnumerator allEntriesEnum = EnumerateAllEntries();
            NativeParser curParser = allEntriesEnum.GetNext();
            while (!curParser.IsNull())
            {
                entries[curParser.Offset] = curParser.LowHashcode;
                curParser = allEntriesEnum.GetNext();
            }
            entries[_endOffset] = 0;

            sb.AppendLine($"NativeHashtable Size: {entries.Count - 1}");
            sb.AppendLine($"EntryIndexSize: {_entryIndexSize}");
            int curOffset = -1;
            foreach (KeyValuePair<uint, byte> entry in entries)
            {
                int nextOffset = (int)entry.Key;
                if (curOffset != -1)
                {
                    for (int i = curOffset; i < nextOffset; i++)
                    {
                        sb.Append($"{_imageReader[i]:X2} ");
                    }
                    sb.AppendLine();
                }
                if (nextOffset != _endOffset)
                {
                    sb.Append($"0x{entry.Value:X2} -> ");
                }
                curOffset = nextOffset;
            }

            return sb.ToString();
        }

        //
        // The enumerator does not conform to the regular C# enumerator pattern to avoid paying
        // its performance penalty (allocation, multiple calls per iteration)
        //
        public struct Enumerator
        {
            private NativeParser _parser;
            private uint _endOffset;
            private byte _lowHashcode;

            internal Enumerator(NativeParser parser, uint endOffset, byte lowHashcode)
            {
                _parser = parser;
                _endOffset = endOffset;
                _lowHashcode = lowHashcode;
            }
        }

        public struct AllEntriesEnumerator
        {
            private NativeHashtable _table;
            private NativeParser _parser;
            private uint _currentBucket;
            private uint _endOffset;

            internal AllEntriesEnumerator(NativeHashtable table)
            {
                _table = table;
                _currentBucket = 0;
                _parser = _table.GetParserForBucket(_currentBucket, out _endOffset);
            }

            public NativeParser GetNext()
            {
                for (; ; )
                {
                    while (_parser.Offset < _endOffset)
                    {
                        return _parser.GetParserFromRelativeOffset();
                    }

                    if (_currentBucket >= _table._bucketMask)
                        return new NativeParser();

                    _currentBucket++;
                    _parser = _table.GetParserForBucket(_currentBucket, out _endOffset);
                }
            }
        }

        private NativeParser GetParserForBucket(uint bucket, out uint endOffset)
        {
            uint start, end;

            if (_entryIndexSize == 0)
            {
                int bucketOffset = (int)(_baseOffset + bucket);
                start = _imageReader.ReadByte(ref bucketOffset);
                end = _imageReader.ReadByte(ref bucketOffset);
            }
            else if (_entryIndexSize == 1)
            {
                int bucketOffset = (int)(_baseOffset + 2 * bucket);
                start = _imageReader.ReadUInt16(ref bucketOffset);
                end = _imageReader.ReadUInt16(ref bucketOffset);
            }
            else
            {
                int bucketOffset = (int)(_baseOffset + 4 * bucket);
                start = _imageReader.ReadUInt32(ref bucketOffset);
                end = _imageReader.ReadUInt32(ref bucketOffset);
            }

            endOffset = end + _baseOffset;
            return new NativeParser(_imageReader, _baseOffset + start);
        }

        public Enumerator Lookup(int hashcode)
        {
            uint endOffset;
            uint bucket = ((uint)hashcode >> 8) & _bucketMask;
            NativeParser parser = GetParserForBucket(bucket, out endOffset);

            return new Enumerator(parser, endOffset, (byte)hashcode);
        }

        public AllEntriesEnumerator EnumerateAllEntries()
        {
            return new AllEntriesEnumerator(this);
        }
    }

    /// <summary>
    /// based on <a href="https://github.com/dotnet/runtime/blob/main/src/coreclr/vm/nativeformatreader.h">NativeFormat::NativeHashtable</a>
    /// </summary>
    public struct NativeCuckooFilter
    {
        // TODO (refactoring) - all these Native* class should be private
        private NativeReader _imageReader;
        private int _filterStartOffset;
        private int _filterEndOffset;

        public NativeCuckooFilter(NativeReader imageReader, int filterStartOffset, int filterEndOffset)
        {
            _imageReader = imageReader;
            _filterStartOffset = filterStartOffset;
            _filterEndOffset = filterEndOffset;

            if (((_filterStartOffset & 0xF) != 0) || ((_filterEndOffset & 0xF) != 0))
            {
                // Native cuckoo filters must be aligned at 16byte boundaries within the PE file
                throw new System.BadImageFormatException();
            }
        }

        private IEnumerable<ushort[]> GetBuckets()
        {
            int offset = _filterStartOffset;
            while (offset < _filterEndOffset)
            {
                ushort[] bucket = new ushort[8];
                for (int i = 0; i < bucket.Length; i++)
                {
                    bucket[i] = _imageReader.ReadUInt16(ref offset);
                }
                yield return bucket;
            }
        }

        public override string ToString()
        {
            StringBuilder sb = new StringBuilder();

            sb.AppendLine($"NativeCuckooFilter Size: {(_filterEndOffset - _filterStartOffset) / 16}");
            int bucket = 0;
            foreach (ushort[] bucketContents in GetBuckets())
            {
                sb.Append($"Bucket: {bucket} [");
                for (int i = 0; i < 8; i++)
                {
                    sb.Append($"{bucketContents[i],4:X} ");
                }
                sb.AppendLine("]");
                bucket++;
            }

            return sb.ToString();
        }
    }
}
