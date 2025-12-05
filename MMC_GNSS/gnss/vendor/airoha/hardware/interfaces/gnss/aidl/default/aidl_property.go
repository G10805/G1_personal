package airoha

import (
    "android/soong/android"
    "android/soong/cc"
    "fmt"
    "time"
)

func init() {
    fmt.Println("==========================")
    fmt.Println("#  AIDL  GNSS Control    #")
    fmt.Println("==========================")
    android.RegisterModuleType("airoha_aidl_default", propertyControl)
}
func propertyControl() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, propertyControlProc)
    return module
}
func propertyControlProc(ctx android.LoadHookContext) {
    fmt.Println("==========================")
    fmt.Println("Build Time: " + time.Now().Format("2006-01-02 15:04:05") + " Version: HAS_PROPERTY_CONTROL_FEATURE")
    fmt.Println("Build:" + ctx.ModuleName())
    fmt.Println("Build Platform Version:" + ctx.Config().PlatformVersionName())
    fmt.Println("Build SDK API Version:" + ctx.Config().PlatformSdkVersion().String())
    // api := ctx.Config().PlatformSdkVersion().String()
    type props struct {
        Shared_libs []string
        Cflags      []string
        Vintf_fragments []string
    }
    p := &props{}
    platformVer := ctx.Config().PlatformVersionName()
    aidlUse := ctx.Config().Getenv("USE_GNSS_AIDL")
    if aidlUse != "" {
        platformVer = aidlUse
    }
    fmt.Println("Select AIDL Platform Version:" + platformVer)
    if platformVer == "14" {
        p.Shared_libs = append(p.Shared_libs, "android.hardware.gnss-V3-ndk")
        p.Cflags = append(p.Cflags, "-DAIR_ANDROID_14_IMPL")
        p.Vintf_fragments = append(p.Vintf_fragments, "android.hardware.gnss-service.airoha.v3.xml")
    } else if platformVer == "VanillaIceCream" {
        p.Shared_libs = append(p.Shared_libs, "android.hardware.gnss-V4-ndk")
        p.Cflags = append(p.Cflags, "-DAIR_ANDROID_15_IMPL")
        p.Cflags = append(p.Cflags, "-DAIR_ANDROID_14_IMPL")
        p.Vintf_fragments = append(p.Vintf_fragments, "android.hardware.gnss-service.airoha.v4.xml")
    } else if platformVer == "13" {
        p.Shared_libs = append(p.Shared_libs, "android.hardware.gnss-V2-ndk")
        p.Vintf_fragments = append(p.Vintf_fragments, "android.hardware.gnss-service.airoha.v2.xml")
    } 
    fmt.Printf("Cflags with: %+v\n", p.Cflags)
    fmt.Printf("Link with: %+v\n", p.Shared_libs)
    fmt.Printf("XML use: %+v\n", p.Vintf_fragments)
    fmt.Println("==========================")
    ctx.AppendProperties(p)
}