namespace DrAnalyzer.Analyzer.Info
{
    public class GatheredError : GatheredInfo
    {
        public override System.Drawing.Color Color { get; } = System.Drawing.Color.Red;
        protected override string Title { get; } = "Error";
        
        public GatheredError(GatherType type, GatherFuncType funcType, string description = null)
            : base(type, funcType, description) { }
    }
}