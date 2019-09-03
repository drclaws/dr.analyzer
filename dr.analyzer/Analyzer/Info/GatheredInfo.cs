namespace DrAnalyzer.Analyzer.Info
{
    public abstract class GatheredInfo
    {
        public GatherType Type { get; }
        
        private GatherFuncType FuncType { get; }
        
        public string Description { get; }
        
        public abstract System.Drawing.Color Color { get; }
        protected abstract string Title { get; }
        
        protected GatheredInfo(GatherType type, GatherFuncType funcType, string description = null)
        {
            this.Type = type;
            this.FuncType = funcType;
            this.Description = description;
        }
        
        public string AsTextMessage()
        {
            string message = $"{this.Title}: {this.Type.GetDescription()}";
            if (this.Description != null)
            {
                message += $" - {this.Description}";
            }
            return message +  $" (from {this.FuncType.GetDescription()})";
        }
    }
}
