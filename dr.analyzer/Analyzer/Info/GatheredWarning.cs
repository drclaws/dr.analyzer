namespace DrAnalyzer.Analyzer.Info
{
    public class GatheredWarning : GatheredInfo
    {
        public override System.Drawing.Color Color { get; } = System.Drawing.Color.Yellow;

        protected override string Title { get; } = "Warning";
        
        public GatheredWarning(GatherType type, GatherFuncType funcType)
            : base(type, funcType) { }

        public GatheredWarning(GatherType type, GatherFuncType funcType, string description)
            : base(type, funcType, description) { }
    }
}
