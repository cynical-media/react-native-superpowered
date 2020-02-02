package com.x86kernel.rnsuperpowered;

public class SuperpoweredJni {
    static {
        System.loadLibrary("SuperpoweredJni");
    }

    private static native void init();

}
