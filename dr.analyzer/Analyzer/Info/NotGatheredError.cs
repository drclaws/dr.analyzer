namespace DrAnalyzer.Analyzer.Info
{
    public class NotGatheredError : GatheredError
    {
        public NotGatheredError() 
            : base(GatherType.GatherStopped, GatherFuncType.GatherReceiverThread, "Connection with the process lost") { }
    }
}
