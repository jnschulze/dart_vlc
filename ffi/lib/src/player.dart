import 'dart:io';
import 'dart:async';
import 'dart:typed_data';
import 'package:dart_vlc_ffi/src/enums/playbackState.dart';
import 'package:ffi/ffi.dart';
import 'package:dart_vlc_ffi/dart_vlc_ffi.dart';
import 'package:dart_vlc_ffi/src/equalizer.dart';
import 'package:dart_vlc_ffi/src/internal/ffi.dart';
import 'package:dart_vlc_ffi/src/playerState/playerState.dart';
import 'package:dart_vlc_ffi/src/mediaSource/media.dart';
import 'package:dart_vlc_ffi/src/mediaSource/mediaSource.dart';
import 'package:dart_vlc_ffi/src/device.dart';

enum PixelFormat { kNone, kRGBA, kBGRA }

/// Represents dimensions of a video.
class VideoDimensions {
  /// Width of the video.
  final int width;

  /// Height of the video.
  final int height;
  const VideoDimensions({this.width = 0, this.height = 0});

  @override
  String toString() => '($width, $height)';
}

/// Represents a [Video] frame, used for retriving frame through platform channel.
class VideoFrame {
  final int width;
  final int height;
  final Uint8List buffer;
  final int bytesPerRow;
  final PixelFormat pixelFormat;

  VideoFrame({
    required this.width,
    required this.height,
    required this.bytesPerRow,
    required this.buffer,
    required this.pixelFormat,
  });
}

mixin _StreamControllers {
  final _videoDimensionsStreamController =
      StreamController<VideoDimensions>.broadcast();
  final _currentMediaStateController =
      StreamController<CurrentMediaState>.broadcast();
  final _positionStateController = StreamController<PositionState>.broadcast();
  final _playbackStateController = StreamController<PlaybackStatus>.broadcast();
  final _generalStateController = StreamController<GeneralState>.broadcast();
  final _videoFrameController = StreamController<VideoFrame>.broadcast();

  CurrentMediaState _currentMediaState = CurrentMediaState();
  PositionState _positionState = PositionState();
  PlaybackStatus _playbackState = PlaybackStatus();
  GeneralState _generalState = GeneralState();
  VideoDimensions _videoDimensions = VideoDimensions();

  /// State of the current & opened [MediaSource] in [Player] instance.
  CurrentMediaState get currentMedia => _currentMediaState;

  /// Stream to listen to current & opened [MediaSource] state of the [Player] instance.
  Stream<CurrentMediaState> get currentMediaStream =>
      _currentMediaStateController.stream;

  /// Position & duration state of the [Player] instance.
  PositionState get position => _positionState;

  /// Stream to listen to position & duration state of the [Player] instance.
  Stream<PositionState> get positionStream => _positionStateController.stream;

  /// Playback state of the [Player] instance.
  PlaybackStatus get playback => _playbackState;

  /// Stream to listen to playback state of the [Player] instance.
  Stream<PlaybackStatus> get playbackStream => _playbackStateController.stream;

  /// Volume & Rate state of the [Player] instance.
  GeneralState get general => _generalState;

  /// Stream to listen to volume & rate state of the [Player] instance.
  Stream<GeneralState> get generalStream => _generalStateController.stream;

  /// Dimensions of the currently playing video.
  VideoDimensions get videoDimensions => _videoDimensions;

  /// Stream to listen to dimensions of currently playing video.
  Stream<VideoDimensions> get videoDimensionsStream =>
      _videoDimensionsStreamController.stream;

  /// Stream for listening to video frames.
  Stream<VideoFrame> get videoFrameStream => _videoFrameController.stream;

  void _closeControllers() {
    _videoDimensionsStreamController.close();
    _videoFrameController.close();
    _currentMediaStateController.close();
    _positionStateController.close();
    _playbackStateController.close();
    _generalStateController.close();
  }
}

/// A [Player] to open & play a [Media] or [Playlist] from file, network or asset.
///
/// Use [Player] constructor to create a new instance of a [Player].
///
/// ```dart
/// Player player = Player();
/// ```
///
/// Use various methods & event streams available to control & listen to events of the playback.
///
class Player with _StreamControllers {
  bool _isDisposed = false;

  /// Id associated with the [Player] instance.
  late int id;

  /// Commandline arguments passed to this instance of [Player].
  final List<String>? commandlineArguments;

  /// Creates a new [Player] instance.
  ///
  /// ```dart
  /// Player player = Player();
  /// ```
  ///
  Player({VideoDimensions? videoDimensions, this.commandlineArguments}) {
    final args = commandlineArguments ?? [];
    id = PlayerFFI.create(
      this.videoDimensions.width,
      this.videoDimensions.width,
      args.length,
      args.toNativeUtf8Array(),
    );
    PlatformEventProxy().register(id, _handlePlatformEvent);
  }

  /// Opens a new media source into the player.
  ///
  /// Takes a [Media] or [Playlist] to open in the player.
  ///
  /// Starts playback itself by default. Pass `autoStart: false` to stop this from happening.
  ///
  /// * Open a new [Media].
  ///
  /// ```dart
  /// player.open(
  ///   Media.file(
  ///     new File('C:/music.ogg'),
  ///   ),
  ///   autoStart: false,
  /// );
  /// ```
  ///
  /// * Open a new [Playlist].
  ///
  /// ```dart
  /// player.open(
  ///   new Playlist(
  ///     medias: [
  ///       Media.file(
  ///         new File('C:/music.mp3'),
  ///       ),
  ///       Media.file(
  ///         new File('C:/audio.mp3'),
  ///       ),
  ///       Media.network('https://alexmercerind.github.io/music.mp3'),
  ///     ],
  ///   ),
  /// );
  /// ```
  ///
  void open(MediaSource source, {bool autoStart: true}) {
    if (source is Media) {
      PlayerFFI.open(
          id,
          autoStart ? 1 : 0,
          <String>[source.mediaType.toString(), source.resource]
              .toNativeUtf8Array(),
          1);
    }
    if (source is Playlist) {
      List<String> medias = <String>[];
      source.medias.forEach((media) {
        medias.add(media.mediaType.toString());
        medias.add(media.resource);
      });
      PlayerFFI.open(id, autoStart ? 1 : 0, medias.toNativeUtf8Array(),
          source.medias.length);
    }
  }

  /// Plays opened [MediaSource],
  void play() {
    PlayerFFI.play(id);
  }

  /// Pauses opened [MediaSource],
  void pause() {
    PlayerFFI.pause(id);
  }

  /// Play or Pause opened [MediaSource],
  void playOrPause() {
    PlayerFFI.playOrPause(id);
  }

  /// Stops the [Player].
  ///
  /// Also resets the [Device] set using [Player.setDevice].
  /// A new instance must be created, once this method is called.
  ///
  void stop() {
    PlayerFFI.stop(id);
  }

  /// Jumps to the next [Media] in the [Playlist] opened.
  void next() {
    PlayerFFI.next(id);
  }

  /// Jumps to the previous [Media] in the [Playlist] opened.
  void back() {
    PlayerFFI.back(id);
  }

  /// Jumps to [Media] at specific index in the [Playlist] opened.
  /// Pass index as parameter.
  void jump(int index) {
    PlayerFFI.jump(id, index);
  }

  /// Seeks the [Media] currently playing in the [Player] instance, to the provided [Duration].
  void seek(Duration duration) {
    PlayerFFI.seek(id, duration.inMilliseconds);
  }

  /// Sets volume of the [Player] instance.
  void setVolume(double volume) {
    PlayerFFI.setVolume(id, volume);
  }

  /// Sets playback rate of the [Media] currently playing in the [Player] instance.
  void setRate(double rate) {
    PlayerFFI.setRate(id, rate);
  }

  /// Sets user agent for dart_vlc player.
  void setUserAgent(String userAgent) {
    PlayerFFI.setUserAgent(id, userAgent.toNativeUtf8());
  }

  /// Changes [Playlist] playback mode.
  void setPlaylistMode(PlaylistMode playlistMode) {
    PlayerFFI.setPlaylistMode(id, playlistMode.toString().toNativeUtf8());
  }

  /// Appends [Media] to the [Playlist] of the [Player] instance.
  void add(Media source) {
    PlayerFFI.add(id, source.mediaType.toString().toNativeUtf8(),
        source.resource.toString().toNativeUtf8());
  }

  /// Removes [Media] from the [Playlist] at a specific index.
  void remove(int index) {
    PlayerFFI.remove(id, index);
  }

  /// Inserts [Media] to the [Playlist] of the [Player] instance at specific index.
  void insert(int index, Media source) {
    PlayerFFI.insert(id, index, source.mediaType.toString().toNativeUtf8(),
        source.resource.toString().toNativeUtf8());
  }

  /// Moves [Media] already present in the [Playlist] of the [Player] from [initialIndex] to [finalIndex].
  void move(int initialIndex, int finalIndex) {
    PlayerFFI.move(id, initialIndex, finalIndex);
  }

  /// Sets playback [Device] for the instance of [Player].
  ///
  /// Use [Devices.all] getter to get [List] of all [Device].
  ///
  /// A playback [Device] for a [Player] instance cannot be changed in the middle of playback.
  /// Device will be switched once a new [Media] is played.
  ///
  void setDevice(Device device) {
    PlayerFFI.setDevice(
        id, device.id.toNativeUtf8(), device.name.toNativeUtf8());
  }

  /// Sets [Equalizer] for the [Player].
  void setEqualizer(Equalizer equalizer) {
    PlayerFFI.setEqualizer(id, equalizer.id);
  }

  /// Saves snapshot of a video to a desired [File] location.
  void takeSnapshot(File file, int width, int height) {
    PlayerFFI.takeSnapshot(id, file.path.toNativeUtf8(), width, height);
  }

  void _handlePlatformEvent(EventKind kind, dynamic data) {
    if (_isDisposed) {
      return;
    }

    switch (kind) {
      case EventKind.videoFrameAvailable:
        final buffer = data[0] as Uint8List;
        final width = data[1] as int;
        final height = data[2] as int;
        final pixelFormat = PixelFormat.values[data[3]];
        final bytesPerRow = data[4] as int;
        _videoFrameController.add(VideoFrame(
            buffer: buffer,
            width: width,
            height: height,
            pixelFormat: pixelFormat,
            bytesPerRow: bytesPerRow));
        break;
      case EventKind.positionChanged:
        final position = data[0] as double;
        assert(position >= 0 && position <= 1);
        _positionState = _positionState.copyWith(
            relativePosition: position,
            duration: Duration(milliseconds: data[1]));
        _positionStateController.sink.add(_positionState);
        break;
      case EventKind.playbackStateChanged:
        _playbackState = _playbackState.copyWith(
            state: PlaybackState.values[data[0]], isSeekable: data[1]);
        _playbackStateController.sink.add(_playbackState);
        print("last state is ${_playbackState.playbackState}");
        break;
      case EventKind.playlistUpdated:
        final currentMediaIndex = data[0] as int;
        final isPlaylist = data[1] as bool;
        final mediaTypes = data[2] as List<dynamic>;
        final mediaUrls = data[3] as List<dynamic>;

        assert(mediaTypes.length == mediaUrls.length);

        List<Media> medias = [];
        for (int i = 0; i < mediaTypes.length; i++) {
          final url = mediaUrls[i];
          switch (mediaTypes[i]) {
            case 'MediaType.file':
              {
                medias.add(Media.file(File(url)));
                break;
              }
            case 'MediaType.network':
              {
                medias.add(Media.network(Uri.parse(url)));
                break;
              }
            case 'MediaType.directShow':
              {
                medias.add(Media.directShow(rawUrl: url));
                break;
              }
          }
        }

        final activeMedia = medias[currentMediaIndex];
        _currentMediaState = _currentMediaState.copyWith(
            medias: medias,
            isPlaylist: isPlaylist,
            index: currentMediaIndex,
            media: activeMedia);
        _currentMediaStateController.add(_currentMediaState);
        break;
      case EventKind.videoDimensionsChanged:
        _videoDimensions = VideoDimensions(width: data[0], height: data[1]);
        _videoDimensionsStreamController.sink.add(_videoDimensions);
        break;
      case EventKind.volumeChanged:
        _generalState = _generalState.copyWith(volume: data[0] as double);
        _generalStateController.sink.add(_generalState);
        break;
      case EventKind.rateChanged:
        _generalState = _generalState.copyWith(rate: data[0] as double);
        _generalStateController.sink.add(_generalState);
        break;
      default:
        return;
    }
  }

  /// Destroys the instance of [Player] & closes all [StreamController]s in it.
  void dispose() {
    if (!_isDisposed) {
      _isDisposed = true;
      _closeControllers();
      PlayerFFI.dispose(id);
    }
  }
}
