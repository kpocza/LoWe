namespace LoWeExposer.Handlers
{
    class MiceState
    {
        public bool LeftButtonDown { get; set; }

        public bool RightButtonDown { get; set; }

        public int X { get; set; }
        public int Y { get; set; }

        public void Reset()
        {
            LeftButtonDown = false;
            RightButtonDown = false;
        }

        public MiceState Clone()
        {
            return new MiceState
            {
                X = X,
                Y = Y,
                LeftButtonDown = LeftButtonDown,
                RightButtonDown = RightButtonDown
            };
        }
    }
}
