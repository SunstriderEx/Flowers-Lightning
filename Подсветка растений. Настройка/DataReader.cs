using System;

namespace Подсветка_растений.Настройка
{
    public static class DataReader
    {
        public static string DataToTime(byte[] data)
        {
            string time = "Ошибка";
            
            if (data.Length > 0)
            {
                time = RtcCounterToTime(BitConverter.ToUInt32(data, 0));

                /*var dataString = System.Text.Encoding.UTF8.GetString(data);
                var timeParts = dataString.Split(':');
                if (timeParts.Length == 3) // Часы, минуты и секунды
                {
                    string[] formattedTimeParts = new string[3];
                    for (int i = 0; i < formattedTimeParts.Length; i++)
                        formattedTimeParts[i] = timeParts[i].Length == 1 ? "0" + timeParts[i] : timeParts[i];

                    time = string.Format("{0}:{1}:{2}", formattedTimeParts[0], formattedTimeParts[1], formattedTimeParts[2]);
                }*/
            }

            return time;
        }

        private static string RtcCounterToTime(uint rtcCounter)
        {
            uint time;
            uint t1;
            string hour;
            string min;
            string sec;

            time = rtcCounter;
            t1 = time / 60;
            sec = (time - t1 * 60).ToString();
            sec = sec.Length == 1 ? "0" + sec : sec;

            time = t1;
            t1 = time / 60;
            min = (time - t1 * 60).ToString();
            min = min.Length == 1 ? "0" + min : min;

            time = t1;
            t1 = time / 24;
            hour = (time - t1 * 24).ToString();
            hour = hour.Length == 1 ? "0" + hour : hour;

            return string.Format("{0}:{1}:{2}", hour, min, sec);
        }

        private static byte[] TimeToData(DateTime time)
        {
            return BitConverter.GetBytes(TimeToRtcCounter(time));
        }

        private static int TimeToRtcCounter(DateTime time)
        {
            int JDN = 0;

            JDN += (time.Hour * 3600);
            JDN += (time.Minute * 60);
            JDN += (time.Second);

            return JDN;
        }

        public static byte[] TimeToOutputData(DateTime time, ReportID timeType)
        {
            byte[] timeData = TimeToData(time);
            byte[] data = new byte[timeData.Length + 2];
            data[0] = 0; // первый байт теряется почему-то
            data[1] = (byte) timeType;
            for (int i = 0; i < timeData.Length; i++)
                data[i + 2] = timeData[i];
            return data;
        }
    }
}