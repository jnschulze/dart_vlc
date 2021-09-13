import 'dart:ffi';
import 'dart:isolate';
import 'package:ffi/ffi.dart';
import 'package:dart_vlc_ffi/src/internal/dynamiclibrary.dart';
import 'package:dart_vlc_ffi/src/internal/typedefs/player.dart';
import 'package:dart_vlc_ffi/src/internal/typedefs/media.dart';
import 'package:dart_vlc_ffi/src/internal/typedefs/devices.dart';
import 'package:dart_vlc_ffi/src/internal/typedefs/equalizer.dart';
import 'package:dart_vlc_ffi/src/internal/typedefs/record.dart';
import 'package:dart_vlc_ffi/src/internal/typedefs/broadcast.dart';
import 'package:dart_vlc_ffi/src/internal/typedefs/chromecast.dart';

enum EventKind {
  unknown,
  playlistUpdated,
  playbackStateChanged,
  positionChanged,
  rateChanged,
  volumeChanged,
  videoDimensionsChanged,
  videoFrameAvailable
}

typedef EventCallback = void Function(EventKind event, dynamic data);

class PlatformEventProxy {
  final ReceivePort receiver = ReceivePort();
  final Map<int, EventCallback> _callbacks = Map<int, EventCallback>();
  static final PlatformEventProxy _instance = PlatformEventProxy._create();

  PlatformEventProxy._create() {
    receiver.asBroadcastStream().listen((event) {
      final args = event as List<dynamic>;
      final id = args[0];
      final kind = EventKind.values[args[1]];
      _callbacks[id]?.call(kind, args[2]);
    });
  }

  factory PlatformEventProxy() {
    return _instance;
  }

  void register(int id, EventCallback callback) {
    _callbacks[id] = callback;
  }

  void unregister(int id) {
    _callbacks.remove(id);
  }
}

abstract class PlayerFFI {
  static final PlayerCreateDart create = dynamicLibrary
      .lookup<NativeFunction<PlayerCreateCXX>>('PlayerCreate')
      .asFunction();

  static final PlayerDisposeDart dispose = dynamicLibrary
      .lookup<NativeFunction<PlayerDisposeCXX>>('PlayerDispose')
      .asFunction();

  static final PlayerOpenDart open = dynamicLibrary
      .lookup<NativeFunction<PlayerOpenCXX>>('PlayerOpen')
      .asFunction();

  static final PlayerTriggerDart play = dynamicLibrary
      .lookup<NativeFunction<PlayerTriggerCXX>>('PlayerPlay')
      .asFunction();

  static final PlayerTriggerDart pause = dynamicLibrary
      .lookup<NativeFunction<PlayerTriggerCXX>>('PlayerPause')
      .asFunction();

  static final PlayerTriggerDart playOrPause = dynamicLibrary
      .lookup<NativeFunction<PlayerTriggerCXX>>('PlayerPlayOrPause')
      .asFunction();

  static final PlayerTriggerDart stop = dynamicLibrary
      .lookup<NativeFunction<PlayerTriggerCXX>>('PlayerStop')
      .asFunction();

  static final PlayerTriggerDart next = dynamicLibrary
      .lookup<NativeFunction<PlayerTriggerCXX>>('PlayerNext')
      .asFunction();

  static final PlayerTriggerDart back = dynamicLibrary
      .lookup<NativeFunction<PlayerTriggerCXX>>('PlayerBack')
      .asFunction();

  static final PlayerJumpDart jump = dynamicLibrary
      .lookup<NativeFunction<PlayerJumpCXX>>('PlayerJump')
      .asFunction();

  static final PlayerSeekDart seek = dynamicLibrary
      .lookup<NativeFunction<PlayerJumpCXX>>('PlayerSeek')
      .asFunction();

  static final PlayerSetVolumeDart setVolume = dynamicLibrary
      .lookup<NativeFunction<PlayerSetVolumeCXX>>('PlayerSetVolume')
      .asFunction();

  static final PlayerSetRateDart setRate = dynamicLibrary
      .lookup<NativeFunction<PlayerSetRateCXX>>('PlayerSetRate')
      .asFunction();

  static final PlayerSetUserAgentDart setUserAgent = dynamicLibrary
      .lookup<NativeFunction<PlayerSetUserAgentCXX>>('PlayerSetUserAgent')
      .asFunction();

  static final PlayerSetEqualizerDart setEqualizer = dynamicLibrary
      .lookup<NativeFunction<PlayerSetEqualizerCXX>>('PlayerSetEqualizer')
      .asFunction();

  static final PlayerSetDeviceDart setDevice = dynamicLibrary
      .lookup<NativeFunction<PlayerSetDeviceCXX>>('PlayerSetDevice')
      .asFunction();

  static final PlayerSetPlaylistModeDart setPlaylistMode = dynamicLibrary
      .lookup<NativeFunction<PlayerSetPlaylistModeCXX>>('PlayerSetPlaylistMode')
      .asFunction();

  static final PlayerAddDart add = dynamicLibrary
      .lookup<NativeFunction<PlayerAddCXX>>('PlayerAdd')
      .asFunction();

  static final PlayerRemoveDart remove = dynamicLibrary
      .lookup<NativeFunction<PlayerRemoveCXX>>('PlayerRemove')
      .asFunction();

  static final PlayerInsertDart insert = dynamicLibrary
      .lookup<NativeFunction<PlayerInsertCXX>>('PlayerInsert')
      .asFunction();

  static final PlayerMoveDart move = dynamicLibrary
      .lookup<NativeFunction<PlayerMoveCXX>>('PlayerMove')
      .asFunction();

  static final PlayerTakeSnapshotDart takeSnapshot = dynamicLibrary
      .lookup<NativeFunction<PlayerTakeSnapshotCXX>>('PlayerTakeSnapshot')
      .asFunction();
}

abstract class MediaFFI {
  static final MediaParseDart parse = dynamicLibrary
      .lookup<NativeFunction<MediaParseCXX>>('MediaParse')
      .asFunction();
}

abstract class BroadcastFFI {
  static final BroadcastCreateDart create = dynamicLibrary
      .lookup<NativeFunction<BroadcastCreateCXX>>('BroadcastCreate')
      .asFunction();

  static final BroadcastStartDart start = dynamicLibrary
      .lookup<NativeFunction<BroadcastStartCXX>>('BroadcastStart')
      .asFunction();

  static final BroadcastDisposeDart dispose = dynamicLibrary
      .lookup<NativeFunction<BroadcastDisposeCXX>>('BroadcastDispose')
      .asFunction();
}

abstract class ChromecastFFI {
  static final ChromecastCreateDart create = dynamicLibrary
      .lookup<NativeFunction<ChromecastCreateCXX>>('ChromecastCreate')
      .asFunction();

  static final ChromecastStartDart start = dynamicLibrary
      .lookup<NativeFunction<ChromecastStartCXX>>('ChromecastStart')
      .asFunction();

  static final ChromecastDisposeDart dispose = dynamicLibrary
      .lookup<NativeFunction<ChromecastDisposeCXX>>('ChromecastDispose')
      .asFunction();
}

abstract class RecordFFI {
  static final RecordCreateDart create = dynamicLibrary
      .lookup<NativeFunction<RecordCreateCXX>>('RecordCreate')
      .asFunction();

  static final RecordStartDart start = dynamicLibrary
      .lookup<NativeFunction<RecordStartCXX>>('RecordStart')
      .asFunction();

  static final RecordDisposeDart dispose = dynamicLibrary
      .lookup<NativeFunction<RecordDisposeCXX>>('RecordDispose')
      .asFunction();
}

abstract class DevicesFFI {
  static final DevicesAllDart all = dynamicLibrary
      .lookup<NativeFunction<DevicesAllCXX>>('DevicesAll')
      .asFunction();
}

abstract class EqualizerFFI {
  static final EqualizerCreateEmptyDart createEmpty = dynamicLibrary
      .lookup<NativeFunction<EqualizerCreateEmptyCXX>>('EqualizerCreateEmpty')
      .asFunction();

  static final EqualizerCreateModeDart createMode = dynamicLibrary
      .lookup<NativeFunction<EqualizerCreateModeCXX>>('EqualizerCreateMode')
      .asFunction();

  static final EqualizerSetBandAmpDart setBandAmp = dynamicLibrary
      .lookup<NativeFunction<EqualizerSetBandAmpCXX>>('EqualizerSetBandAmp')
      .asFunction();

  static final EqualizerSetPreAmpDart setPreAmp = dynamicLibrary
      .lookup<NativeFunction<EqualizerSetPreAmpCXX>>('EqualizerSetPreAmp')
      .asFunction();
}

bool isInitialized = false;

extension NativeTypes on List<String> {
  Pointer<Pointer<Utf8>> toNativeUtf8Array() {
    final List<Pointer<Utf8>> listPointer = this
        .map((String string) => string.toNativeUtf8())
        .toList()
        .cast<Pointer<Utf8>>();
    final Pointer<Pointer<Utf8>> pointerPointer =
        calloc.allocate(this.join('').length);
    for (int index = 0; index < this.length; index++)
      pointerPointer[index] = listPointer[index];
    return pointerPointer;
  }
}
