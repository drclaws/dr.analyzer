using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DrAnalyzer.Analyzer.Info
{
    public class GatheredResource : IGatheredInfo
    {
        public GatherType Type { get; private set; }
        public GatherFuncType FuncType { get; private set; }
        public string Name { get; private set; }

        public GatheredResource(GatherType type, GatherFuncType funcType, string name)
        {
            this.Type = type;
            this.FuncType = funcType;
            this.Name = name;
        }

        public string AsTextMessage()
        {
            return String.Format("{0} ({1}): {2}", this.Type.GetDescription(), this.FuncType.GetDescription(), this.Name);
        }
    }
}
