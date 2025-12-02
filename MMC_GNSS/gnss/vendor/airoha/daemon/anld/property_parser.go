package anld_property_control

import (
    "android/soong/android"
    "android/soong/cc"
    "android/soong/etc"
    "fmt"
    "time"
)
var AirohaExtGnssInterface = ""
func init() {
    fmt.Println("==========================")
    fmt.Println("#     Property Control    #")
    fmt.Println("==========================")
    android.RegisterModuleType("anld_property_control", propertyControl)
    android.RegisterModuleType("xml_include", xmlIncludeControl)
    android.RegisterModuleType("extgnss_cc_static_default", extgnssStaticDefaultControl)
    android.RegisterModuleType("extgnss_cc_binary_default", extgnssBinaryDefaultControl)
    android.RegisterModuleType("extgnss_cc_service_default", extgnssServiceDefaultControl)
}
func propertyControl() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, propertyControlProc)
    return module
}
func xmlIncludeControl() android.Module {
    module := etc.PrebuiltEtcFactory()
    // module.properties.Src
    android.AddLoadHook(module, xmlIncludeControlProc)
    return module
}
func extgnssStaticDefaultControl() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, extgnssStaticDefaultControlProc)
    return module
}
func extgnssBinaryDefaultControl() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, extgnssBinaryDefaultControlProc)
    return module
}
func extgnssServiceDefaultControl() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, extgnssServiceDefaultControlProc)
    return module
}
func xmlIncludeControlProc(ctx android.LoadHookContext) {
    fmt.Println("==========================")
    fmt.Println("xml include: " + ctx.ModuleName())
    fmt.Println("==========================")
    type prop_xml_src struct {
        Src *string
    }
    if ctx.ModuleName() == "anld_user_setting.xml" {
        factory_xml_name := "anld_user_setting_factory.xml"
        user_xml_name := "anld_user_setting.xml"
        p := &prop_xml_src{}
        var factory_key string;
        factory_key = ctx.Config().Getenv("CUSTOMER_FACTORY_MACRO");
        fmt.Println("Factory Key:" + factory_key);
        if factory_key == "" {
            fmt.Println("factory_key not found");
            if ctx.Config().Debuggable(){
                fmt.Println("debuggable mode, use factory.xml");
                p.Src = &factory_xml_name;
            } else {
                fmt.Println("user mode, use user.xml");
                p.Src = &user_xml_name;
            }
        } else if factory_key == "true" {
            fmt.Println("factory_key true, use factory.xml");
            p.Src = &factory_xml_name;
        } else if factory_key == "false" {
            fmt.Println("factory_key false, use user.xml");
            p.Src = &user_xml_name;
        }
        ctx.AppendProperties(p)
    }
    
}
func propertyControlProc(ctx android.LoadHookContext) {
    fmt.Println("==========================")
    fmt.Println("Build Time: " + time.Now().Format("2006-01-02 15:04:05") + " Version: HAS_PROPERTY_CONTROL_FEATURE")
    fmt.Println("==========================")
}

func extgnssStaticDefaultControlProc(ctx android.LoadHookContext) {
    // api := ctx.Config().PlatformSdkVersion().String()
    type props struct {
        Shared_libs []string
        Cflags      []string
    }
    p := &props{}
    platformVer := ctx.Config().PlatformVersionName()
    extGnssUse := ctx.Config().Getenv("HAS_AIROHA_EXTGNSS")
    fmt.Printf("extGnssUse:" + extGnssUse)
    if extGnssUse == "" {
        extGnssUse = AirohaExtGnssInterface
    }
    if extGnssUse != "" {
        p.Cflags = append(p.Cflags, "-DHAS_AIROHA_EXTGNSS")
    }
    if extGnssUse == "1.0" {
        p.Cflags = append(p.Cflags, "-DHAS_AIROHA_EXTGNSS_HIDL_V1_0")
        p.Shared_libs = append(p.Shared_libs, "vendor.airoha.hardware.extgnss@1.0")
    }
    fmt.Printf("platformVer:" + platformVer)
    fmt.Printf("Cflags with: %+v\n", p.Cflags)
    fmt.Printf("Link with: %+v\n", p.Shared_libs)

    fmt.Println("==========================")
    ctx.AppendProperties(p)
}
func extgnssBinaryDefaultControlProc(ctx android.LoadHookContext) {
    // api := ctx.Config().PlatformSdkVersion().String()
    type props struct {

        Static_libs []string

    }
    p := &props{}
    platformVer := ctx.Config().PlatformVersionName()
    extGnssUse := ctx.Config().Getenv("HAS_AIROHA_EXTGNSS")
    if extGnssUse == "" {
        extGnssUse = AirohaExtGnssInterface
    }
    if extGnssUse != "" {
        p.Static_libs = append(p.Static_libs, "vendor.airoha.hardware.extgnss@static-airoha")
    }
    fmt.Printf("platformVer:" + platformVer)
    fmt.Printf("Static_libs with: %+v\n", p.Static_libs)

    fmt.Println("==========================")
    ctx.AppendProperties(p)
}
func extgnssServiceDefaultControlProc(ctx android.LoadHookContext) {
    // api := ctx.Config().PlatformSdkVersion().String()
    type props struct {
        Vintf_fragments []string
    }
    p := &props{}
    platformVer := ctx.Config().PlatformVersionName()
    extGnssUse := ctx.Config().Getenv("HAS_AIROHA_EXTGNSS")
    if extGnssUse == "" {
        extGnssUse = AirohaExtGnssInterface
    }
    if extGnssUse != "" {
        p.Vintf_fragments = append(p.Vintf_fragments, "vendor.android.hardware.extgnss@1.0-service-airoha.xml")
    }
    fmt.Printf("platformVer:" + platformVer)

    fmt.Printf("XML use: %+v\n", p.Vintf_fragments)
    fmt.Println("==========================")
    ctx.AppendProperties(p)
}
