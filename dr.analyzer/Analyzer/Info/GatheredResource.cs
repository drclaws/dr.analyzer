namespace DrAnalyzer.Analyzer.Info
{
    public class GatheredResource : GatheredInfo
    {
        public override System.Drawing.Color Color { get; } = System.Drawing.Color.Black;
        protected override string Title { get; } = "Resource";

        public GatheredResource(GatherType type, GatherFuncType funcType, string filepath)
            : base(type, funcType, filepath) { }
    }
}
