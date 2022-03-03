import 'dart:async';

import 'package:flutter/services.dart';

class ScreenShot {
  static const MethodChannel _channel = MethodChannel('screen_shot');

  static Future<bool?> tackScreenshot(
      {required int dx,
      required int dy,
      required int width,
      required int high,
      String imageName = "screenshot",
      required String imagePath}) async {
    final bool? version = await _channel.invokeMethod('screenshot', {
      'x1': dx.abs(),
      'y1': dy.abs(),
      'w': width.abs(),
      'h': high.abs(),
      'path': imagePath,
      'imageName': imageName
    });
    return version;
  }
}
