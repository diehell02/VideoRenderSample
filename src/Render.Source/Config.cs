using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Text.Json;

namespace Render.Source
{
    public class Config
    {
        private const string FILE_NAME = "Config.json";

        public static Config Instance { get; private set; }

        static Config()
        {
            try
            {
                string content = File.ReadAllText($"{AppDomain.CurrentDomain.BaseDirectory}/{FILE_NAME}");
                Config? config = JsonSerializer.Deserialize<Config>(content);
                if (config is null)
                {
                    Instance = CreateDefault();
                }
                else
                {
                    Instance = config;
                }
            }
            catch
            {
                Instance = CreateDefault();
            }
            if (Instance.LocalYuvFile?.YuvPath == null)
            {
                Instance.LocalYuvFile = null;
            }
            Instance.LocalYuvFile?.Init();
            if (Instance.YuvFiles is null)
            {
                return;
            }
            Instance.YuvFiles.Sort((x, y) =>
            {
                int xArea = x.FrameWidth * x.FrameHeight;
                int yArea = y.FrameHeight * y.FrameWidth;
                if (xArea < yArea)
                {
                    return -1;
                }
                else if (yArea == xArea)
                {
                    return 0;
                }
                else
                {
                    return 1;
                }
            });
            foreach (var yuvFile in Instance.YuvFiles)
            {
                yuvFile.Init();
            }
        }

        public static Config CreateDefault()
        {
            return new Config()
            {
                YuvFiles = new List<YuvFile>()
                {
                    new YuvFile()
                    {
                        YuvPath = "240x134.yuv",
                        FrameWidth = 240,
                        FrameHeight = 134,
                        FrameLength = 100,
                    }
                },
                FramePerSecond = 30,
                IsMultipleThread = true,
                IsMultipleScreen = false,
                AutoChangeLocaltion = false,
                VideoViewNumber = 1,
            };
        }

        public List<YuvFile>? YuvFiles { get; set; }
        public YuvFile? LocalYuvFile { get; set; }
        public int FramePerSecond { get; set; }

        public bool IsMultipleThread { get; set; }

        public bool IsMultipleScreen { get; set; }

        public bool AutoChangeLocaltion { get; set; }

        public int VideoViewNumber { get; set; }

        public void Save()
        {
            var json = JsonSerializer.Serialize(this);
            File.WriteAllText(FILE_NAME, json);
        }
    }
}
