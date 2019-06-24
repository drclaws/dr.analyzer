using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DrAnalyzer.Analyzer.Info
{
    class GatheredStatus : IGatheredInfo
    {
        public GatherType Type { get; private set; }
        public GatherFuncType FuncType { get; private set; }
        public string Name
        {
            get
            {
                switch (this.Type)
                {
                    case GatherType.GatherActivated:
                        return "Gathering activated";
                    case GatherType.GatherStillUp:
                        return "Gathering still up";
                    case GatherType.GatherDeactivated:
                        return "Gathering stopped";
                    default:
                        return "Unknown status";
                }
            }
        }

        public GatheredStatus(GatherType type, GatherFuncType funcType)
        {
            this.Type = type;
            this.FuncType = funcType;
        }

        public string AsTextMessage()
        {
            return String.Format("{0} ({1})", this.Type.GetDescription(), this.FuncType.GetDescription());
        }
    }
}
