// P5: user-reported disappearing Mix and airless wet signal.
// Numerical gates: dry onset, 50% constant-power law, wet existence,
// mono fold, controlled shelf frequency selectivity and sweep continuity.
// Sound quality still needs real user listening and host validation.
#include <arverb/Reverb.hpp>
#include "../src/dsp/Primitives.hpp"
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

static void check(bool ok,const char* why) {
    if (!ok) {std::fprintf(stderr,"MIX/AIR FAILURE: %s\n",why);std::abort();}
}
static double shelfGain(float hz,float air,float fs) {
    arverb::detail::AirShelf shelf;
    const float coefficient=arverb::detail::lowPassCoeff(6500.0f,fs);
    double before=0,after=0;
    for(int i=0;i<48000;++i) {
        float s=0.3f*std::sin(2.0f*arverb::detail::kPi*hz*i/fs);
        float out=shelf.process(s,coefficient,air);
        if(i>24000) {before+=s*s;after+=out*out;}
    }
    return std::sqrt(after/before);
}
static double render(int mode,float mix,bool mono) {
    arverb::Engine e;
    e.loadPreset(arverb::Preset::VocalPlate);
    e.setAlgorithm(static_cast<arverb::Algorithm>(mode));
    e.setParameter(arverb::Parameter::Mix,mix);
    e.setParameter(arverb::Parameter::DuckAmount,0.0f);
    e.prepare(48000.0);
    std::array<float,128> l{},r{},a{},b{};
    double sum=0.0;
    for(int block=0;block<600;++block) {
        for(int i=0;i<128;++i) {
            float t=static_cast<float>(block*128+i);
            l[i]=0.25f*std::sin(t*0.061f)+0.09f*std::sin(t*0.72f);
            r[i]=mono?l[i]:0.22f*std::sin(t*0.041f+0.7f);
        }
        e.process(l.data(),r.data(),a.data(),b.data(),128);
        for(int i=0;i<128;++i) {
            check(std::isfinite(a[i])&&std::isfinite(b[i]),"finite all mixes/modes");
            if(block>200) {
                const float collapsed=0.5f*(a[i]+b[i]);
                sum+=static_cast<double>(collapsed)*collapsed;
            }
        }
    }
    return std::sqrt(sum/(399.0*128.0));
}
int main() {
    // These must use the exact gain law; with 36ms predelay, the first
    // sample after prepare is dry only, not phase-cancelled wet energy.
    for(float mix:{0.f,0.5f,1.f}) {
        arverb::Engine e;
        e.setParameter(arverb::Parameter::Mix,mix);
        e.setParameter(arverb::Parameter::PreDelayMs,60.f);
        e.prepare(48000.0);
        float l=0.5f,r=0.3f,a=0,b=0;
        e.process(&l,&r,&a,&b,1);
        const float expected=mix==0.f?1.f:mix==0.5f?0.70710678f:0.f;
        check(std::abs(a-0.5f*expected)<0.00005f,
              "50% Mix must retain -3dB dry, not old -6dB dip");
        check(std::abs(b-0.3f*expected)<0.00005f,
              "Mix gain matched L and R at wet onset");
    }
    check(shelfGain(10500.f,0.5f,48000.f)>
          shelfGain(250.f,0.5f,48000.f)+0.12,
          "wet air shelf lifts high frequency more than bass");
    check(std::abs(shelfGain(8000.f,0.f,48000.f)-1.0)<1e-8,
          "air shelf at zero lift exactly transparent");

    for(int mode=0;mode<4;++mode) {
        const double dry=render(mode,0.f,false);
        check(dry>0.07,"healthy stereo dry benchmark");
        double prev=render(mode,0.f,true);
        for(float mix:{0.25f,0.5f,0.75f,1.f}) {
            const double rms=render(mode,mix,true);
            std::printf("mode=%d mix=%.2f collapsed_mono_rms=%.6f dry=%.6f\n",
                        mode,mix,rms,dry);
            check(rms>0.014,"wet-heavy Mix should not go near silence in mono");
            check(rms<dry*3.4,"wet-heavy Mix must avoid excessive output gain");
            check(std::fabs(rms-prev)<dry*1.5,
                  "Mix sweep no extreme abrupt level jump");
            prev=rms;
        }
    }
    std::puts("PASS P5: equal-power dry/wet, non-silent mono wet, air selectivity");
}
