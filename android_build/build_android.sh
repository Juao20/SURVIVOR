#!/usr/bin/env bash
# Build an installable Android APK for SURVIVOR using raylib + Android NDK, no Gradle.
set -e

# ---- Paths (adjust NDK if the extracted folder name differs) ----
GAME_ROOT="/c/Users/HP/SURVIVOR"
AB="$GAME_ROOT/android_build"
RAYLIB_SRC="$AB/raylib/src"
SDK="/c/Users/HP/AppData/Local/Android/Sdk"
NDK="$SDK/ndk-r27c"
BUILD_TOOLS="$SDK/build-tools/37.0.0"
PLATFORM_JAR="$SDK/platforms/android-36/android.jar"
JAVA_HOME_DIR="/c/Program Files/Android/Android Studio/jbr"
JAVAC="$JAVA_HOME_DIR/bin/javac.exe"

TOOLCHAIN="$NDK/toolchains/llvm/prebuilt/windows-x86_64"
NATIVE_GLUE="$NDK/sources/android/native_app_glue"
ANDROID_API=29
ARCH_NAME=arm64-v8a
CC="$TOOLCHAIN/bin/aarch64-linux-android$ANDROID_API-clang"
CXX="$TOOLCHAIN/bin/aarch64-linux-android$ANDROID_API-clang++"

APP_COMPANY=survivor
APP_PRODUCT=game
APP_LABEL="Survivor"
PROJECT_LIB=main
KEYSTORE_PASS=android123

OUT="$AB/build"
rm -rf "$OUT"
mkdir -p "$OUT/obj" "$OUT/lib/$ARCH_NAME" "$OUT/res/values" "$OUT/res/drawable-mdpi" \
         "$OUT/assets/assets" "$OUT/src/com/$APP_COMPANY/$APP_PRODUCT" "$OUT/classes" "$OUT/dex" "$OUT/bin"

echo "== [1/9] Building libraylib.a for Android (arm64) =="
( cd "$RAYLIB_SRC" && \
  PATH="$TOOLCHAIN/bin:$PATH" make -j4 OS=Windows_NT PLATFORM=PLATFORM_ANDROID ANDROID_NDK="$NDK" \
      ANDROID_ARCH=arm64 ANDROID_API_VERSION=$ANDROID_API RAYLIB_LIBTYPE=STATIC )

echo "== [2/9] Compiling android_native_app_glue.c =="
"$CC" -c "$NATIVE_GLUE/android_native_app_glue.c" -o "$OUT/obj/native_app_glue.o" \
  -I"$NATIVE_GLUE" --sysroot="$TOOLCHAIN/sysroot" -DANDROID -fPIC -O2

echo "== [3/9] Compiling game as libmain.so =="
"$CXX" -std=c++17 -fPIC -shared -O2 -static-libstdc++ \
  -o "$OUT/lib/$ARCH_NAME/lib$PROJECT_LIB.so" \
  "$GAME_ROOT/src/main.cpp" "$OUT/obj/native_app_glue.o" \
  -I"$GAME_ROOT" -I"$RAYLIB_SRC" -I"$NATIVE_GLUE" \
  -DPLATFORM_ANDROID \
  -L"$RAYLIB_SRC" -lraylib \
  -landroid -llog -lEGL -lGLESv2 -lOpenSLES -ldl -lm -lc \
  -Wl,-u,ANativeActivity_onCreate \
  --sysroot="$TOOLCHAIN/sysroot"

echo "== [4/9] Copying assets (double-nested to match code's 'assets/...' paths) =="
cp -r "$GAME_ROOT/assets/." "$OUT/assets/assets/"

echo "== [5/9] Writing resources (icon + strings.xml) =="
cp "$AB/raylib/logo/raylib_48x48.png" "$OUT/res/drawable-mdpi/icon.png"
cat > "$OUT/res/values/strings.xml" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<resources><string name="app_name">$APP_LABEL</string></resources>
EOF

echo "== [6/9] Writing NativeLoader.java + AndroidManifest.xml =="
cat > "$OUT/src/com/$APP_COMPANY/$APP_PRODUCT/NativeLoader.java" <<EOF
package com.$APP_COMPANY.$APP_PRODUCT;

public class NativeLoader extends android.app.NativeActivity {
    static {
        System.loadLibrary("$PROJECT_LIB");
    }

    @Override
    protected void onCreate(android.os.Bundle savedInstanceState) {
        // Render under the camera-cutout notch so the game uses the full
        // screen width in landscape instead of leaving a black bar.
        if (android.os.Build.VERSION.SDK_INT >= 28) {
            getWindow().getAttributes().layoutInDisplayCutoutMode =
                android.view.WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
        }
        super.onCreate(savedInstanceState);
    }
}
EOF

cat > "$OUT/AndroidManifest.xml" <<EOF
<?xml version="1.0" encoding="utf-8"?>
<manifest xmlns:android="http://schemas.android.com/apk/res/android"
        package="com.$APP_COMPANY.$APP_PRODUCT"
        android:versionCode="1" android:versionName="1.0" >
    <uses-sdk android:minSdkVersion="$ANDROID_API" android:targetSdkVersion="$ANDROID_API" />
    <uses-feature android:glEsVersion="0x00020000" android:required="true" />
    <application android:allowBackup="false" android:label="@string/app_name" android:icon="@drawable/icon" >
        <activity android:name="com.$APP_COMPANY.$APP_PRODUCT.NativeLoader"
            android:theme="@android:style/Theme.NoTitleBar.Fullscreen"
            android:configChanges="orientation|keyboardHidden|screenSize"
            android:screenOrientation="landscape" android:launchMode="singleTask"
            android:exported="true"
            android:clearTaskOnLaunch="true">
            <meta-data android:name="android.app.lib_name" android:value="$PROJECT_LIB" />
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>
</manifest>
EOF

echo "== [7/9] aapt: generate R.java, compile Java, dex =="
"$BUILD_TOOLS/aapt.exe" package -f -m -S "$OUT/res" -J "$OUT/src" -M "$OUT/AndroidManifest.xml" -I "$PLATFORM_JAR"

"$JAVAC" -d "$OUT/classes" -classpath "$PLATFORM_JAR" -sourcepath "$OUT/src" \
  "$OUT/src/com/$APP_COMPANY/$APP_PRODUCT/R.java" \
  "$OUT/src/com/$APP_COMPANY/$APP_PRODUCT/NativeLoader.java"

JAVA_HOME="$JAVA_HOME_DIR" PATH="$JAVA_HOME_DIR/bin:$PATH" "$BUILD_TOOLS/d8.bat" --output "$OUT/bin" $(find "$OUT/classes" -name "*.class")

echo "== [8/9] Package + sign APK =="
"$BUILD_TOOLS/aapt.exe" package -f -M "$OUT/AndroidManifest.xml" -S "$OUT/res" -A "$OUT/assets" \
  -I "$PLATFORM_JAR" -F "$OUT/bin/app.unsigned.apk" "$OUT/bin"

( cd "$OUT" && "$BUILD_TOOLS/aapt.exe" add "bin/app.unsigned.apk" "lib/$ARCH_NAME/lib$PROJECT_LIB.so" )

if [ ! -f "$AB/survivor.keystore" ]; then
  "$JAVA_HOME_DIR/bin/keytool.exe" -genkeypair -validity 10000 \
    -dname "CN=Survivor,O=Dev,C=FR" -keystore "$AB/survivor.keystore" \
    -storepass $KEYSTORE_PASS -keypass $KEYSTORE_PASS -alias survivorkey -keyalg RSA -keysize 2048
fi

"$BUILD_TOOLS/zipalign.exe" -f 4 "$OUT/bin/app.unsigned.apk" "$OUT/bin/app.aligned.apk"

JAVA_HOME="$JAVA_HOME_DIR" PATH="$JAVA_HOME_DIR/bin:$PATH" "$BUILD_TOOLS/apksigner.bat" sign --ks "$AB/survivor.keystore" --ks-pass pass:$KEYSTORE_PASS \
  --out "$AB/Survivor.apk" "$OUT/bin/app.aligned.apk"

echo "== [9/9] Done: $AB/Survivor.apk =="
ls -la "$AB/Survivor.apk"
