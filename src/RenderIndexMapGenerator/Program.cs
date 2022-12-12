using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace RenderIndexMapGenerator
{
    internal class Program
    {
        static void Main(string[] args)
        {
            var result = CalculateStdDev(new List<double>() { 1, 1, 1, -1, 1, 1, -1, 1, 1, 1 });

            uint length = 30;

            var logPath = "log.txt";
            File.Delete(logPath);
            Trace.Listeners.Add(new TextWriterTraceListener(logPath));
            for (uint i = 1; i <= length; i++)
            {
                var array = Generate(i, length);
                string template = "{ 0, new int[2] {0, 1} },";
                StringBuilder outputText = new StringBuilder();
                outputText.Append("{ ").Append(i).Append($", new int[{length}]").Append(" { ");
                for (int j = 0; j < array.Length; j++)
                {
                    outputText.Append(array[j]);
                    if (j < array.Length - 1)
                    {
                        outputText.Append(",");
                    }
                }
                outputText.Append(" } },");
                Trace.WriteLine(outputText);
                Console.WriteLine("Finish " + i);
            }
            Trace.Flush();
            Debug.Flush();

            Console.WriteLine("Hello World!");
        }

        static int[] Generate(uint count, uint length)
        {
            int[] targetArray = new int[length];
            for (int index = 0; index < length; index++)
            {
                targetArray[index] = -1;
            }
            for (int index = 0; index < count; index++)
            {
                targetArray[index] = 1;
            }
            for (int i = 0; i < 100000; i++)
            {
                targetArray = Swap(targetArray, count, length);
            }
            return targetArray;
        }

        static double GetInterval(int[] targetArray)
        {
            List<int> intervalList = new List<int>();
            int firstIndex = -1;
            int flag = -1;
            if (targetArray.Count(item => item == 1) > targetArray.Length / 2)
            {
                flag = 1;
            }
            for (int index = 0; index < targetArray.Length; index++)
            {
                int currentIndex = index;
                int nextIndex = 0;
                if (targetArray[currentIndex] == flag)
                {
                    continue;
                }
                if (firstIndex == -1)
                {
                    firstIndex = currentIndex;
                }
                for (int index1 = index + 1; index1 < targetArray.Length + 1; index1++)
                {
                    if (index1 == targetArray.Length)
                    {
                        nextIndex = targetArray.Length + firstIndex;
                        break;
                    }
                    if (targetArray[index1] == flag)
                    {
                        continue;
                    }
                    else
                    {
                        nextIndex = index1;
                        break;
                    }
                }
                intervalList.Add(nextIndex - currentIndex);
            }
            if (intervalList.Count == 0)
            {
                return 1;
            }
            return StandardDeviation(intervalList);
        }

        static double StandardDeviation(List<int> intervalList)
        {
            double average = intervalList.Average();
            double sum = intervalList.Sum(interval =>
            {
                double d = interval - average;
                return Math.Pow(Math.Abs(d), 2);
            });
            return Math.Sqrt(sum / intervalList.Count);
        }

        private static double CalculateStdDev(IEnumerable<double> values)
        {
            double ret = 0;
            if (values.Count() > 0)
            {
                //  计算平均数   
                double avg = values.Average();
                //  计算各数值与平均数的差值的平方，然后求和 
                double sum = values.Sum(d => Math.Pow(d - avg, 2));
                //  除以数量，然后开方
                ret = Math.Sqrt(sum / values.Count());
            }
            return ret;
        }

        static int[] Swap(int[] targetArray, uint count, uint length)
        {
            //decimal expectInterval = length / (decimal)count;
            // 计算间距
            double currentInterval = GetInterval(targetArray);
            // 拷贝数组
            int[] tempArray = new int[length];
            targetArray.CopyTo(tempArray, 0);
            // 交换随机两个元素
            int swapIndex1 = RandomNumberGenerator.GetInt32(0, (int)length);
            int swapIndex2 = RandomNumberGenerator.GetInt32(0, (int)length);
            int tempValue = tempArray[swapIndex1];
            tempArray[swapIndex1] = tempArray[swapIndex2];
            tempArray[swapIndex2] = tempValue;
            // 计算间距
            double swapInterval = GetInterval(tempArray);

            // 对比是否进步, 返回结果更优的数据
            if (currentInterval < swapInterval)
            {
                return targetArray;
            }
            else
            {
                return tempArray;
            }
        }
    }
}
