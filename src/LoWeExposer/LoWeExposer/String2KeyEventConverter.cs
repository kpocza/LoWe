using System.Collections.Generic;
using System.Linq;
using System.Windows.Input;

namespace LoWeExposer
{
    internal static class String2KeyEventConverter
    {
        class KeyEvt
        {
            private Key _key;
            private bool _shift;

            internal static KeyEvt Press(Key key)
            {
                return new KeyEvt {_key = key, _shift = false};
            }

            internal static KeyEvt ShiftPress(Key key)
            {
                return new KeyEvt { _key = key, _shift = true};
            }

            internal IEnumerable<KeyEventArgs> ToEvents()
            {
                if (_shift)
                {
                    yield return ToKeyDownEvent(Key.LeftShift);
                }

                yield return ToKeyDownEvent(_key);
                yield return ToKeyUpEvent(_key);

                if (_shift)
                {
                    yield return ToKeyUpEvent(Key.LeftShift);
                }
            }

            private static KeyEventArgs ToKeyDownEvent(Key key)
            {
                return new KeyEventArgs(Keyboard.PrimaryDevice, Keyboard.PrimaryDevice.ActiveSource, 0, key)
                {
                    RoutedEvent = Keyboard.KeyDownEvent
                };
            }

            private static KeyEventArgs ToKeyUpEvent(Key key)
            {
                return new KeyEventArgs(Keyboard.PrimaryDevice, Keyboard.PrimaryDevice.ActiveSource, 0, key)
                {
                    RoutedEvent = Keyboard.KeyUpEvent
                };
            }
        }

        internal static IEnumerable<KeyEventArgs> ToEvents(string str)
        {
            return str.ToCharArray().Where(c => _evtDictionary.ContainsKey(c)).SelectMany(c => _evtDictionary[c].ToEvents());
        }

        private static readonly IDictionary<char, KeyEvt> _evtDictionary = new Dictionary<char, KeyEvt>
        {
            {'`', KeyEvt.Press(Key.Oem3)},
            {'1', KeyEvt.Press(Key.D1)},
            {'2', KeyEvt.Press(Key.D2)},
            {'3', KeyEvt.Press(Key.D3)},
            {'4', KeyEvt.Press(Key.D4)},
            {'5', KeyEvt.Press(Key.D5)},
            {'6', KeyEvt.Press(Key.D6)},
            {'7', KeyEvt.Press(Key.D7)},
            {'8', KeyEvt.Press(Key.D8)},
            {'9', KeyEvt.Press(Key.D9)},
            {'0', KeyEvt.Press(Key.D0)},
            {'-', KeyEvt.Press(Key.OemMinus)},
            {'=', KeyEvt.Press(Key.OemPlus)},
            {'\t', KeyEvt.Press(Key.Tab)},
            {'q', KeyEvt.Press(Key.Q)},
            {'w', KeyEvt.Press(Key.W)},
            {'e', KeyEvt.Press(Key.E)},
            {'r', KeyEvt.Press(Key.R)},
            {'t', KeyEvt.Press(Key.T)},
            {'y', KeyEvt.Press(Key.Y)},
            {'u', KeyEvt.Press(Key.U)},
            {'i', KeyEvt.Press(Key.I)},
            {'o', KeyEvt.Press(Key.O)},
            {'p', KeyEvt.Press(Key.P)},
            {'[', KeyEvt.Press(Key.Oem4)},
            {']', KeyEvt.Press(Key.Oem6)},
            {'a', KeyEvt.Press(Key.A)},
            {'s', KeyEvt.Press(Key.S)},
            {'d', KeyEvt.Press(Key.D)},
            {'f', KeyEvt.Press(Key.F)},
            {'g', KeyEvt.Press(Key.G)},
            {'h', KeyEvt.Press(Key.H)},
            {'j', KeyEvt.Press(Key.J)},
            {'k', KeyEvt.Press(Key.K)},
            {'l', KeyEvt.Press(Key.L)},
            {';', KeyEvt.Press(Key.OemSemicolon)},
            {'\'', KeyEvt.Press(Key.Oem7)},
            {'\\', KeyEvt.Press(Key.Oem5)},
            {'z', KeyEvt.Press(Key.Z)},
            {'x', KeyEvt.Press(Key.X)},
            {'c', KeyEvt.Press(Key.C)},
            {'v', KeyEvt.Press(Key.V)},
            {'b', KeyEvt.Press(Key.B)},
            {'n', KeyEvt.Press(Key.N)},
            {'m', KeyEvt.Press(Key.M)},
            {',', KeyEvt.Press(Key.OemComma)},
            {'.', KeyEvt.Press(Key.OemPeriod)},
            {'/', KeyEvt.Press(Key.Oem2)},
            {'~', KeyEvt.ShiftPress(Key.Oem3) },
            {'!', KeyEvt.ShiftPress(Key.D1)},
            {'@', KeyEvt.ShiftPress(Key.D2)},
            {'#', KeyEvt.ShiftPress(Key.D3)},
            {'$', KeyEvt.ShiftPress(Key.D4)},
            {'%', KeyEvt.ShiftPress(Key.D5)},
            {'^', KeyEvt.ShiftPress(Key.D6)},
            {'&', KeyEvt.ShiftPress(Key.D7)},
            {'*', KeyEvt.ShiftPress(Key.D8)},
            {'(', KeyEvt.ShiftPress(Key.D9)},
            {')', KeyEvt.ShiftPress(Key.D0)},
            {'_', KeyEvt.ShiftPress(Key.OemMinus)},
            {'+', KeyEvt.ShiftPress(Key.OemPlus) },
            {'Q', KeyEvt.ShiftPress(Key.Q)},
            {'W', KeyEvt.ShiftPress(Key.W)},
            {'E', KeyEvt.ShiftPress(Key.E)},
            {'R', KeyEvt.ShiftPress(Key.R)},
            {'T', KeyEvt.ShiftPress(Key.T)},
            {'Y', KeyEvt.ShiftPress(Key.Y)},
            {'U', KeyEvt.ShiftPress(Key.U)},
            {'I', KeyEvt.ShiftPress(Key.I)},
            {'O', KeyEvt.ShiftPress(Key.O)},
            {'P', KeyEvt.ShiftPress(Key.P)},
            {'{', KeyEvt.ShiftPress(Key.Oem4)},
            {'}', KeyEvt.ShiftPress(Key.Oem6)},
            {'A', KeyEvt.ShiftPress(Key.A)},
            {'S', KeyEvt.ShiftPress(Key.S)},
            {'D', KeyEvt.ShiftPress(Key.D)},
            {'F', KeyEvt.ShiftPress(Key.F)},
            {'G', KeyEvt.ShiftPress(Key.G)},
            {'H', KeyEvt.ShiftPress(Key.H)},
            {'J', KeyEvt.ShiftPress(Key.J)},
            {'K', KeyEvt.ShiftPress(Key.K)},
            {'L', KeyEvt.ShiftPress(Key.L)},
            {':', KeyEvt.ShiftPress(Key.OemSemicolon) },
            {'"', KeyEvt.ShiftPress(Key.Oem7) },
            {'|', KeyEvt.ShiftPress(Key.Oem5) },
            {'Z', KeyEvt.ShiftPress(Key.Z)},
            {'X', KeyEvt.ShiftPress(Key.X)},
            {'C', KeyEvt.ShiftPress(Key.C)},
            {'V', KeyEvt.ShiftPress(Key.V)},
            {'B', KeyEvt.ShiftPress(Key.B)},
            {'N', KeyEvt.ShiftPress(Key.N)},
            {'M', KeyEvt.ShiftPress(Key.M)},
            {'<', KeyEvt.ShiftPress(Key.OemComma)},
            {'>', KeyEvt.ShiftPress(Key.OemPeriod)},
            {'?', KeyEvt.ShiftPress(Key.Oem2)},
            {' ', KeyEvt.Press(Key.Space)},
            {'\n', KeyEvt.Press(Key.Enter)}
        };
    }
}
