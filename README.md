# react-native-superpowered

## Goal
The goal of react-native-superpowered is to use a flexible json interpreter (json.hpp) to quickly and easily add support for Superpowered's SDK in a flexible, cross-platform way.

Typical native react-native modules still require quite a bit of duplicate work, reimplementing an API in both objective-C and java.

Here, we implement only 3 things "natively," in java and objective-C
- JSON string receiver and sender
- Microphone/input device access
- Output device access

Everything else is done via generic json command which can be dealt with in C++ using json.hpp

## How to test

```bash
git clone https://github.com/cfogelklou/react-native-superpowered-test.git --recursive
cd superpowered_test
yarn install 
cd ios
pod install
```


#### Install using npm or yarn

```bash
npm install react-native-superpowered --save
```
Or if you're using yarn:

```bash
yarn add react-native-superpowered
```

It takes about 1~2 minutes to complete installation because it clones [SuperpoweredSDK](https://github.com/superpoweredSDK/Low-Latency-Android-iOS-Linux-Windows-tvOS-macOS-Interactive-Audio-Platform/tree/master/Superpowered) and [json](https://github.com/nlohmann/json)
