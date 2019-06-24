using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DrAnalyzer.Analyzer.Info
{
    public class GatheredWarning : IGatheredInfo
    {
        public enum WarningType : Int32
        {
            GatherNameTooBig = -1,
            GatherCannotGetValue = -2
        }

        public GatherType Type { get; private set; }
        public GatherFuncType FuncType { get; private set; }
        public WarningType Warning { get; private set; }
        public string Name
        {
            get
            {
                switch(this.Warning)
                {
                    case WarningType.GatherNameTooBig:
                        return "Name of opened file is to big to transfer";
                    case WarningType.GatherCannotGetValue:
                        return "Cannot get name of file";
                    default:
                        return "Unknown warning";
                }
            }
        }

        public GatheredWarning( GatherType type, GatherFuncType funcType, Int32 warningType)
        {
            this.Type = type;
            this.FuncType = funcType;
            this.Warning = (WarningType)warningType;
        }

        public string AsTextMessage()
        {
            return String.Format("Warning {0} ({1}): {2}", this.Type.GetDescription(), this.FuncType.GetDescription(), this.Name);
        }
    }
}
