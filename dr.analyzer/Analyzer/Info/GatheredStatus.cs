namespace DrAnalyzer.Analyzer.Info
{
    public class GatheredStatus : GatheredInfo
    {
        public override System.Drawing.Color Color { get; } = System.Drawing.Color.Green;

        protected override string Title { get; } = "Status";
        
        public GatheredStatus(GatherType type, GatherFuncType funcType, string description = null)
            : base(type, funcType, description) { }
    }
}
