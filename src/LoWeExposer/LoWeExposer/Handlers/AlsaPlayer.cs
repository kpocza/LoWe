using System;
using System.Collections.Generic;
using System.Linq;
using NAudio.Wave;

namespace LoWeExposer.Handlers
{
    class AlsaPlayer : WaveStream
    {
        private readonly ILineLogger _lineLogger;
        private readonly WaveFormat _waveFormat;
        private readonly object _lockObj;
        private readonly LinkedList<byte[]> _buffers;
        private readonly bool _convertFromFloat;
        private long _length;
        private long _position;

        public AlsaPlayer(int rate, int sampleBits, int channels, int alsaFormat, ILineLogger lineLogger)
        {
            _lineLogger = lineLogger;

            _convertFromFloat = false;
            if (alsaFormat == 16384)
            {
                _convertFromFloat = true;
                sampleBits /= 2;
            }
            _waveFormat = new WaveFormat(rate, sampleBits, channels);
            _lockObj = new object();
            _length = 0;
            _position = 0;
            _buffers = new LinkedList<byte[]>();
        }

        public void Write(byte[] buffer)
        {
            lock (_lockObj)
            {
                if (!_convertFromFloat)
                {
                    _buffers.AddLast(buffer);
                    _length += buffer.Length;
                }
                else
                {
                    var newBuffer = new byte[buffer.Length / 2];
                    int dst = 0;
                    for (int src = 0; src < buffer.Length; src += 4)
                    {
                        float f = BitConverter.ToSingle(buffer, src);
                        var val = (short)(f * 32768);
                        Array.Copy(BitConverter.GetBytes(val), 0, newBuffer, dst, 2);
                        dst += 2;
                    }
                    _buffers.AddLast(newBuffer);
                    _length += newBuffer.Length;
                }
            }

            _lineLogger.LogLine($"Buffering {buffer.Length} bytes");
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            int len = 0;
            lock (_lockObj)
            {
                int left = count;
                int curOffset = offset;
                while (left > 0 && _buffers.Count > 0)
                {
                    var first = _buffers.First();
                    if (first.Length <= left)
                    {
                        Array.Copy(first, 0, buffer, curOffset, first.Length);
                        curOffset += first.Length;
                        left -= first.Length;
                        len += first.Length;
                        _buffers.RemoveFirst();
                    }
                    else
                    {
                        Array.Copy(first, 0, buffer, curOffset, left);
                        Array.Copy(first, left, first, 0, first.Length - left);
                        Array.Resize(ref first, first.Length - left);
                        _buffers.RemoveFirst();
                        _buffers.AddFirst(first);
                        curOffset += left;
                        len += left;
                        left = 0;
                    }
                }

                _position += len;
            }

            _lineLogger.LogLine($"Playing {len} bytes");
            return len;
        }

        public int GetDelay()
        {
            lock (_lockObj)
            {
                int frameBytes = this._waveFormat.BitsPerSample / 8 * _waveFormat.Channels;
                return (int)((Length - Position) / frameBytes);
            }
        }

        public void Drop()
        {
            lock (_lockObj)
            {
                _buffers.Clear();
                _length = 0;
                _position = 0;
            }
        }

        public override WaveFormat WaveFormat => _waveFormat;
        public override long Length => _length;

        public override long Position
        {
            get { return _position; }
            set { _position = value; }
        }
    }
}
