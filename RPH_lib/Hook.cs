using Rage;
using HarmonyLib;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Rage.Exceptions;
using System.Reflection;

[assembly: Rage.Attributes.Plugin("SirenSetting Limit Adjuster", Description = "Allows up to 2^16 siren settings", Author = "cp702")]


namespace SirenSetting_Patcher_RPH
{
    public class Hook
    {
        [DllImport("SirenSetting_Limit_Adjuster.asi")]
        private static extern bool ApplyRphHook();

        /*internal static FieldInfo elAddr;
        internal static MethodInfo elGetBuiltInNativeInstanceByIndex;
        internal static MethodInfo ccGetEmergencyLighting;*/

        public static void Main()
        {
            bool success = ApplyRphHook();
            if (!success)
            {
                Game.DisplayNotification("Error applying hook");
            }
            Game.LogTrivialDebug("hook applied");
/*            elAddr = typeof(EmergencyLighting).GetField("address", BindingFlags.NonPublic | BindingFlags.Instance);
            elGetBuiltInNativeInstanceByIndex = typeof(EmergencyLighting).GetMethod("GetBuiltInNativeInstanceByIndex", BindingFlags.NonPublic | BindingFlags.Static);
            Assembly rph = typeof(EmergencyLighting).Assembly;
            Type ContentCache = rph.GetType("Rage.ContentCache");
            ccGetEmergencyLighting = ContentCache.GetMethod("GetEmergencyLighting");
            Harmony.DEBUG = true;
            var patcher = new Harmony("SirenSetting_Patcher_RPH");
            patcher.PatchAll();
            GameFiber.Hibernate();*/
        }
    }
    /*
    [HarmonyPatch(typeof(Rage.EmergencyLighting))]
    [HarmonyPatch("Id", MethodType.Getter)]
    internal class GetIdPatch
    {
        internal static bool Prefix(EmergencyLighting __instance, ref uint __result)
        {
            if (!__instance.IsValid())
            {
                throw new InvalidHandleableException("Operation is not valid because the specified" + __instance.GetType().BaseType.FullName + "is invalid.");
            }
            IntPtr addr = (IntPtr)Hook.elAddr.GetValue(__instance);
            __result = (uint)Marshal.ReadInt32(addr);
            if (__result == 255)
                __result = uint.MaxValue;
            return false;
        }
    }

    [HarmonyPatch(typeof(Rage.EmergencyLighting))]
    [HarmonyPatch("Id", MethodType.Setter)]
    internal class SetIdPatch
    {
        internal static bool Prefix(uint value, EmergencyLighting __instance)
        {
            if (!__instance.IsValid())
            {
                throw new InvalidHandleableException("Operation is not valid because the specified System.Object is invalid.");
            }
            IntPtr addr = (IntPtr)Hook.elAddr.GetValue(__instance);
            Marshal.WriteInt32(addr, (int)value);
            return false;
        }
    }

    [HarmonyPatch(typeof(Rage.EmergencyLighting))]
    [HarmonyPatch("GetBuiltInFromVehicleModelAddress", new Type[] { typeof(IntPtr) })]
    internal class GetBuiltInPatch
    {
        internal static bool Prefix(IntPtr modelAddress, ref EmergencyLighting __result)
        {
            uint index = 0;
            index += Marshal.ReadByte(IntPtr.Add(modelAddress, 0x53b));
            index += 256u * Marshal.ReadByte(IntPtr.Add(modelAddress, 0x55f));
            IntPtr builtInNativeInstanceByIndex = (IntPtr) Hook.elGetBuiltInNativeInstanceByIndex.Invoke(null, new object[] { index });
            if (builtInNativeInstanceByIndex == null)
            {
                __result = null;
            }
            else
            {
                __result = (EmergencyLighting) Hook.ccGetEmergencyLighting.Invoke(null, new object[] { builtInNativeInstanceByIndex });
            }
            return false;
        }
    }*/
}
