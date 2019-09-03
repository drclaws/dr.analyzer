using System;
using System.Collections.Generic;

namespace DrAnalyzer.Analyzer.Info
{
    public static class InfoBuilder
    {
        private const int TypeSize = sizeof(UInt16);
        private const int FuncTypeSize = sizeof(UInt16);
        private const int LengthSize = sizeof(Int32);
        private const UInt32 LetterSize = sizeof(Char);

        public static List<GatheredInfo> ToInfoType(byte[] message)
        {
            List<GatheredInfo> infoList = new List<GatheredInfo>();

            uint messageSize = Convert.ToUInt32(message.Length);
            uint currPos = 0;

            byte[] buffType = new byte[TypeSize], buffFunc = new byte[FuncTypeSize], buffNameLength = new byte[LengthSize];

            while (currPos < messageSize)
            {
                Array.Copy(message, currPos, buffType, 0, TypeSize);
                currPos += TypeSize;
                Array.Copy(message, currPos, buffFunc, 0, FuncTypeSize);
                currPos += FuncTypeSize;

                GatherType type = (GatherType)BitConverter.ToUInt16(buffType, 0);
                GatherFuncType func = (GatherFuncType)BitConverter.ToInt16(buffFunc, 0);
                string dataInfo = null;
                if (type.HasFlag(GatherType.GatherHasValue))
                {
                    type ^= GatherType.GatherHasValue;
                    Array.Copy(message, currPos, buffNameLength, 0, LengthSize);
                    currPos += LengthSize;
                    UInt32 length = BitConverter.ToUInt32(buffNameLength, 0);
                    
                    byte[] buffName = new byte[length * LetterSize];
                    Array.Copy(message, currPos, buffName, 0, length * LetterSize);
                    currPos += length * LetterSize;
                    dataInfo = System.Text.Encoding.Unicode.GetString(buffName);
                }

                if (type.HasFlag(GatherType.GatherError))
                {
                    infoList.Add(new GatheredError(type, func, dataInfo));
                }
                else if (type.HasFlag(GatherType.GatherWarning))
                {
                    infoList.Add(new GatheredWarning(type, func, dataInfo));
                }
                else if (type.HasFlag(GatherType.GatherResource))
                {
                    infoList.Add(new GatheredResource(type, func, dataInfo));
                }
                else if (type.HasFlag(GatherType.GatherStatus))
                {
                    infoList.Add(new GatheredStatus(type, func, dataInfo));
                }
            }

            return infoList;
        }
    }
}
