using Rage;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Rage.Exceptions;
using System.Reflection;

[assembly: Rage.Attributes.Plugin("SirenSetting Limit Adjuster", Description = "Allows up to 2^16 siren settings", Author = "cp702", PrefersSingleInstance = true)]


namespace SirenSetting_Patcher_RPH
{
    public class Hook
    {
        [DllImport("SirenSetting_Limit_Adjuster.asi")]
        private static extern bool ApplyRphHook();

        public static void Main()
        {
            bool success = ApplyRphHook();
            if (!success)
            {
                Game.DisplayNotification("Error applying hook");
            }
            Game.LogTrivialDebug("hook applied");
        }
    }
}
