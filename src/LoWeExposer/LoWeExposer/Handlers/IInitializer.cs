using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace LoWeExposer.Handlers
{
    interface IInitializer
    {
        IDictionary<string, int> GetResult(ICollection<string> names);
    }
}
