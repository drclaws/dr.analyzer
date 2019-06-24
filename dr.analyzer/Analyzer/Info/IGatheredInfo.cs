using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DrAnalyzer.Analyzer.Info
{
    public interface IGatheredInfo
    { 
        GatherType Type { get; }
        GatherFuncType FuncType { get; }
        string Name { get; }

        string AsTextMessage();

    }
}
