using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DrAnalyzer.Analyzer.Info
{
    public class InfoBuilder
    {
        private const int typeSize = sizeof(UInt16);
        private const int funcTypeSize = sizeof(UInt16);
        private const int lengthSize = sizeof(Int32);
        private const int letterSize = sizeof(Char);

        public InfoBuilder()
        {
            
        }

        public List<GatheredInfo> ToInfoType(byte[] message)
        {
            List<GatheredInfo> infoList = new List<GatheredInfo>();

            int messageSize = Convert.ToInt32(message.Length);
            int currPos = 0;

            GatherType type;
            GatherFuncType func;
            Int32 length;

            byte[] buffName, buffType = new byte[typeSize], buffFunc = new byte[funcTypeSize], buffNameLength = new byte[lengthSize];

            while (currPos < messageSize)
            {
                GatheredInfo info = null;
                Array.Copy(message, currPos, buffType, 0, typeSize);
                currPos += typeSize;
                Array.Copy(message, currPos, buffFunc, 0, funcTypeSize);
                currPos += funcTypeSize;

                type = (GatherType)Convert.ToUInt16(buffType);
                func = (GatherFuncType)Convert.ToInt16(buffFunc);

                if ((type & GatherType.GatherResource) != 0)
                {
                    Array.Copy(message, currPos, buffNameLength, 0, lengthSize);
                    currPos += lengthSize;
                    length = Convert.ToInt32(buffNameLength);

                    if (length > 0)
                    {
                        buffName = new byte[length * letterSize];
                        Array.Copy(message, currPos, buffName, 0, length * letterSize);
                        currPos += length * letterSize;
                        info = new GatheredResource(type, func, System.Text.Encoding.Unicode.GetString(buffName));
                    }
                    else
                    {
                        info = new GatheredWarning(type, func, length);
                    }
                }
                else if ((type & GatherType.GatherStatus) != 0)
                {
                    info = new GatheredStatus(type, func);
                }
                else
                {
                    throw new Exception(String.Format("Error: {0:X4} type doesn't exist", (UInt16)type));
                }
                infoList.Add(info);
            }

            throw new NotImplementedException();
        }
    }
}
