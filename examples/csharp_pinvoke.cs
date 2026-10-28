// C# / .NET example. Build the C ABI DLL first with -DARVERB_BUILD_SHARED=ON.
// Keep one instance per active audio stream; NEVER allocate inside your callback.
// Enable <AllowUnsafeBlocks>true</AllowUnsafeBlocks> in the .csproj.
using System;
using System.Runtime.InteropServices;

internal static unsafe class NativeReverb
{
    [DllImport("arverb", CallingConvention = CallingConvention.Cdecl)]
    internal static extern IntPtr arverb_create();
    [DllImport("arverb", CallingConvention = CallingConvention.Cdecl)]
    internal static extern void arverb_destroy(IntPtr handle);
    [DllImport("arverb", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int arverb_prepare(IntPtr handle, double sampleRate);
    [DllImport("arverb", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int arverb_load_preset(IntPtr handle, uint presetId);
    [DllImport("arverb", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int arverb_set_parameter(IntPtr handle, uint paramId, float value);
    [DllImport("arverb", CallingConvention = CallingConvention.Cdecl)]
    internal static extern void arverb_process(IntPtr handle,
        float* inputLeft, float* inputRight, float* outputLeft, float* outputRight, uint frames);
}

// Host snippet (not a complete audio application):
// IntPtr reverb = NativeReverb.arverb_create();
// NativeReverb.arverb_prepare(reverb, 48000);
// NativeReverb.arverb_load_preset(reverb, 0);  // Vocal Plate preset
// Inside audio callback, with preallocated pinned/managed buffers:
// fixed (float* iL = inputLeft, iR = inputRight, oL = outputLeft, oR = outputRight)
//     NativeReverb.arverb_process(reverb, iL, iR, oL, oR, (uint)frameCount);
// When audio stream is stopped: NativeReverb.arverb_destroy(reverb);
