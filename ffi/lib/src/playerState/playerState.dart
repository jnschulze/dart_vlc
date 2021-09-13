import 'package:dart_vlc_ffi/src/mediaSource/media.dart';
import 'package:dart_vlc_ffi/src/enums/playbackState.dart';

/// State of a [Player] instance.
class CurrentMediaState {
  /// Index of currently playing [Media].
  final int? index;

  /// Currently playing [Media].
  final Media? media;

  /// [List] of [Media] currently opened in the [Player] instance.
  final List<Media> medias;

  /// Whether a [Playlist] is opened or a [Media].
  final bool isPlaylist;

  const CurrentMediaState(
      {this.index,
      this.media,
      this.medias = const [],
      this.isPlaylist = false});

  CurrentMediaState copyWith(
          {int? index, Media? media, List<Media>? medias, bool? isPlaylist}) =>
      CurrentMediaState(
          index: index ?? this.index,
          media: media ?? this.media,
          medias: medias ?? this.medias,
          isPlaylist: isPlaylist ?? this.isPlaylist);
}

/// Position & duration state of a [Player] instance.
class PositionState {
  final double relativePosition;

  /// Position of playback in [Duration] of currently playing [Media].
  Duration get position => duration * relativePosition;

  /// Length of currently playing [Media] in [Duration].
  final Duration duration;

  const PositionState(
      {this.relativePosition = 0.0, this.duration = Duration.zero});

  PositionState copyWith({double? relativePosition, Duration? duration}) =>
      PositionState(
          relativePosition: relativePosition ?? this.relativePosition,
          duration: duration ?? this.duration);
}

/// Playback state of a [Player] instance.
class PlaybackStatus {
  final PlaybackState playbackState;

  /// Whether [Player] instance is seekable or not.
  final bool isSeekable;

  /// Whether [Player] instance is playing or not.
  bool get isPlaying => playbackState == PlaybackState.playing;

  /// Whether the current [Media] has ended playing or not.
  bool get isCompleted => playbackState == PlaybackState.ended;

  const PlaybackStatus(
      {this.playbackState = PlaybackState.none, this.isSeekable = false});

  PlaybackStatus copyWith({PlaybackState? state, bool? isSeekable}) =>
      PlaybackStatus(
          playbackState: state ?? this.playbackState,
          isSeekable: isSeekable ?? this.isSeekable);
}

/// Volume & Rate state of a [Player] instance.
class GeneralState {
  /// Volume of [Player] instance.
  final double volume;

  /// Rate of playback of [Player] instance.
  final double rate;

  const GeneralState({this.volume = 1.0, this.rate = 1.0});

  GeneralState copyWith({double? volume, double? rate}) =>
      GeneralState(volume: volume ?? this.volume, rate: rate ?? this.rate);
}
