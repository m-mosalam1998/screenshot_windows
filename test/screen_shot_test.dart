import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:screen_shot/screen_shot.dart';

void main() {
  const MethodChannel channel = MethodChannel('screen_shot');

  TestWidgetsFlutterBinding.ensureInitialized();

  setUp(() {
    channel.setMockMethodCallHandler((MethodCall methodCall) async {
      return '42';
    });
  });

  tearDown(() {
    channel.setMockMethodCallHandler(null);
  });

  test('tackScreenshot', () async {
    expect(ScreenShot.tackScreenshot, '42');
  });
}
