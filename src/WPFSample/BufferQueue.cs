using System.Collections.Generic;

namespace WPFSample
{
    internal abstract class BufferQueue<T> where T : class
    {
        private readonly object _cacheLock = new object();
        private const int LENGTH = 3;
        protected readonly Queue<T> _cache = new Queue<T>();
        protected readonly Stack<T> _idel = new Stack<T>();
        protected readonly List<T> _bufferList = new List<T>();

        public BufferQueue()
        {
            InitializeCache();
        }

        public void Write(T item)
        {
            lock (_cacheLock)
            {
                if (_idel.TryPop(out T? buffer))
                {
                    CopyIntoBuffer(buffer, item);
                    _cache.Enqueue(buffer);
                }
                else if (_cache.TryDequeue(out buffer))
                {
                    CopyIntoBuffer(buffer, item);
                    _cache.Enqueue(buffer);
                }
            }
        }

        public T? Read()
        {
            T? buffer;
            lock (_cacheLock)
            {
                _cache.TryDequeue(out buffer);
            }
            return buffer;
        }

        public void PutBack(T buffer)
        {
            lock (_cacheLock)
            {
                _idel.Push(buffer);
            }
        }

        public void Clean()
        {
            lock (_cacheLock)
            {
                CleanBuffer();
                _cache.Clear();
                _idel.Clear();
                _bufferList.Clear();
            }
        }

        private void InitializeCache()
        {
            for (var i = 0; i < LENGTH; i++)
            {
                var buffer = CreateCacheItem();
                _idel.Push(buffer);
                _bufferList.Add(buffer);
            }
        }

        protected abstract T CreateCacheItem();

        protected abstract void CopyIntoBuffer(T buffer, T item);

        private void CleanBuffer()
        {
            foreach (var item in _bufferList)
            {
                CleanCacheItem(item);
            }
        }

        protected abstract void CleanCacheItem(T buffer);
    }
}
