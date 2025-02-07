// ignore_for_file: implementation_imports
import 'dart:async';
import 'package:flutter/material.dart';
import 'package:audio_video_progress_bar/audio_video_progress_bar.dart';
import 'package:dart_vlc_ffi/src/device.dart';
import 'package:dart_vlc_ffi/src/player.dart';
import 'package:dart_vlc_ffi/src/playerState/playerState.dart';

class Control extends StatefulWidget {
  final Widget child;
  final int playerId;
  final bool? showTimeLeft;
  final double? progressBarThumbRadius;
  final double? progressBarThumbGlowRadius;
  final Color? progressBarActiveColor;
  final Color? progressBarInactiveColor;
  final Color? progressBarThumbColor;
  final Color? progressBarThumbGlowColor;
  final TextStyle? progressBarTextStyle;
  final Color? volumeActiveColor;
  final Color? volumeInactiveColor;
  final Color? volumeBackgroundColor;
  final Color? volumeThumbColor;

  Control({
    required this.child,
    required this.playerId,
    required this.showTimeLeft,
    required this.progressBarThumbRadius,
    required this.progressBarThumbGlowRadius,
    required this.progressBarActiveColor,
    required this.progressBarInactiveColor,
    required this.progressBarThumbColor,
    required this.progressBarThumbGlowColor,
    required this.progressBarTextStyle,
    required this.volumeActiveColor,
    required this.volumeInactiveColor,
    required this.volumeBackgroundColor,
    required this.volumeThumbColor,
    Key? key,
  }) : super(key: key);

  @override
  ControlState createState() => ControlState();
}

class ControlState extends State<Control> with SingleTickerProviderStateMixin {
  bool _hideControls = true;
  bool _displayTapped = false;
  Timer? _hideTimer;
  late StreamSubscription<PlaybackState> playPauseStream;
  late AnimationController playPauseController;

  @override
  void initState() {
    super.initState();
    this.playPauseController = new AnimationController(
        vsync: this, duration: Duration(milliseconds: 400));
    this.playPauseStream = players[widget.playerId]!
        .playbackStream
        .listen((event) => this.setPlaybackMode(event.isPlaying));
    if (players[widget.playerId]!.playback.isPlaying)
      this.playPauseController.forward();
  }

  @override
  void dispose() {
    this.playPauseStream.cancel();
    super.dispose();
  }

  void setPlaybackMode(bool isPlaying) {
    if (isPlaying)
      this.playPauseController.forward();
    else
      this.playPauseController.reverse();
    this.setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    return GestureDetector(
      onTap: () {
        if (players[widget.playerId]!.playback.isPlaying) {
          if (_displayTapped) {
            setState(() {
              _hideControls = true;
            });
          } else {
            _cancelAndRestartTimer();
          }
        } else {
          setState(() {
            _hideControls = true;
          });
        }
      },
      child: MouseRegion(
        onHover: (_) => _cancelAndRestartTimer(),
        child: AbsorbPointer(
          absorbing: _hideControls,
          child: Stack(
            children: [
              widget.child,
              AnimatedOpacity(
                duration: Duration(milliseconds: 300),
                opacity: _hideControls ? 0.0 : 1.0,
                child: Stack(
                  children: [
                    Container(
                      decoration: BoxDecoration(
                        gradient: LinearGradient(
                          begin: Alignment.topCenter,
                          end: Alignment.bottomCenter,
                          colors: [
                            Color(0xCC000000),
                            Color(0x00000000),
                            Color(0x00000000),
                            Color(0x00000000),
                            Color(0x00000000),
                            Color(0x00000000),
                            Color(0xCC000000),
                          ],
                        ),
                      ),
                    ),
                    Positioned(
                      left: 0,
                      right: 0,
                      bottom: 0,
                      child: Padding(
                        padding:
                            EdgeInsets.only(bottom: 60, right: 20, left: 20),
                        child: StreamBuilder<PositionState>(
                          stream: players[widget.playerId]?.positionStream,
                          builder: (BuildContext context,
                              AsyncSnapshot<PositionState> snapshot) {
                            final durationState = snapshot.data;
                            final progress =
                                durationState?.position ?? Duration.zero;
                            final total =
                                durationState?.duration ?? Duration.zero;
                            return Theme(
                              data: ThemeData.dark(),
                              child: ProgressBar(
                                progress: progress,
                                total: total,
                                barHeight: 3,
                                progressBarColor: widget.progressBarActiveColor,
                                thumbColor: widget.progressBarThumbColor,
                                baseBarColor: widget.progressBarInactiveColor,
                                thumbGlowColor:
                                    widget.progressBarThumbGlowColor,
                                thumbRadius:
                                    widget.progressBarThumbRadius ?? 10.0,
                                thumbGlowRadius:
                                    widget.progressBarThumbGlowRadius ?? 30.0,
                                timeLabelLocation: TimeLabelLocation.sides,
                                timeLabelType: widget.showTimeLeft!
                                    ? TimeLabelType.remainingTime
                                    : TimeLabelType.totalTime,
                                timeLabelTextStyle: widget.progressBarTextStyle,
                                onSeek: (duration) {
                                  players[widget.playerId]!.seek(duration);
                                },
                              ),
                            );
                          },
                        ),
                      ),
                    ),
                    Positioned(
                      left: 0,
                      right: 0,
                      bottom: 10,
                      child: Row(
                        mainAxisSize: MainAxisSize.min,
                        mainAxisAlignment: MainAxisAlignment.center,
                        children: [
                          IconButton(
                            color: Colors.white,
                            iconSize: 30,
                            icon: Icon(Icons.skip_previous),
                            onPressed: () => players[widget.playerId]!.back(),
                          ),
                          SizedBox(width: 50),
                          IconButton(
                              color: Colors.white,
                              iconSize: 30,
                              icon: Icon(Icons.replay_10),
                              onPressed: () {
                                int positionInMilliseconds =
                                    players[widget.playerId]!
                                            .position
                                            .position
                                            ?.inMilliseconds ??
                                        0;
                                if (!(positionInMilliseconds - 10000)
                                    .isNegative)
                                  positionInMilliseconds -= 10000;
                                players[widget.playerId]!.seek(Duration(
                                    milliseconds: positionInMilliseconds));
                                setState(() {});
                              }),
                          SizedBox(width: 20),
                          IconButton(
                            color: Colors.white,
                            iconSize: 30,
                            icon: AnimatedIcon(
                                icon: AnimatedIcons.play_pause,
                                progress: this.playPauseController),
                            onPressed: () {
                              if (players[widget.playerId]!
                                  .playback
                                  .isPlaying) {
                                players[widget.playerId]!.pause();
                                this.playPauseController.reverse();
                              } else {
                                players[widget.playerId]!.play();
                                this.playPauseController.forward();
                              }
                            },
                          ),
                          SizedBox(width: 20),
                          IconButton(
                              color: Colors.white,
                              iconSize: 30,
                              icon: Icon(Icons.forward_10),
                              onPressed: () {
                                int durationInMilliseconds =
                                    players[widget.playerId]!
                                            .position
                                            .duration
                                            ?.inMilliseconds ??
                                        0;
                                int positionInMilliseconds =
                                    players[widget.playerId]!
                                            .position
                                            .position
                                            ?.inMilliseconds ??
                                        1;
                                if ((positionInMilliseconds + 10000) <=
                                    durationInMilliseconds) {
                                  positionInMilliseconds += 10000;
                                  players[widget.playerId]!.seek(Duration(
                                      milliseconds: positionInMilliseconds));
                                  setState(() {});
                                }
                              }),
                          SizedBox(width: 50),
                          IconButton(
                            color: Colors.white,
                            iconSize: 30,
                            icon: Icon(Icons.skip_next),
                            onPressed: () => players[widget.playerId]!.next(),
                          ),
                        ],
                      ),
                    ),
                    Positioned(
                      right: 15,
                      bottom: 12.5,
                      child: Row(
                        crossAxisAlignment: CrossAxisAlignment.end,
                        children: [
                          VolumeControl(
                            playerId: widget.playerId,
                            thumbColor: widget.volumeThumbColor,
                            inactiveColor: widget.volumeInactiveColor,
                            activeColor: widget.volumeActiveColor,
                            backgroundColor: widget.volumeBackgroundColor,
                          ),
                          PopupMenuButton(
                            iconSize: 24,
                            icon: Icon(Icons.speaker, color: Colors.white),
                            onSelected: (Device device) {
                              players[widget.playerId]!.setDevice(device);
                              setState(() {});
                            },
                            itemBuilder: (context) {
                              return Devices.all
                                  .map(
                                    (device) => PopupMenuItem(
                                      child: Text(device.name,
                                          style: TextStyle(
                                            fontSize: 14.0,
                                          )),
                                      value: device,
                                    ),
                                  )
                                  .toList();
                            },
                          ),
                        ],
                      ),
                    ),
                  ],
                ),
              ),
            ],
          ),
        ),
      ),
    );
  }

  void _cancelAndRestartTimer() {
    this._hideTimer?.cancel();

    if (this.mounted) {
      this._startHideTimer();

      this.setState(() {
        _hideControls = false;
        _displayTapped = true;
      });
    }
  }

  void _startHideTimer() {
    this._hideTimer = Timer(const Duration(seconds: 3), () {
      if (this.mounted) {
        this.setState(() {
          _hideControls = true;
        });
      }
    });
  }
}

class VolumeControl extends StatefulWidget {
  final int playerId;
  final Color? activeColor;
  final Color? inactiveColor;
  final Color? backgroundColor;
  final Color? thumbColor;

  const VolumeControl({
    required this.playerId,
    required this.activeColor,
    required this.inactiveColor,
    required this.backgroundColor,
    required this.thumbColor,
    Key? key,
  }) : super(key: key);

  @override
  _VolumeControlState createState() => _VolumeControlState();
}

class _VolumeControlState extends State<VolumeControl> {
  double volume = 0.5;
  bool _showVolume = false;
  double unmutedVolume = 0.5;

  @override
  Widget build(BuildContext context) {
    return Column(
      children: [
        AnimatedOpacity(
          duration: Duration(milliseconds: 250),
          opacity: _showVolume ? 1 : 0,
          child: AbsorbPointer(
            absorbing: !_showVolume,
            child: MouseRegion(
              onEnter: (_) {
                setState(() => _showVolume = true);
              },
              onExit: (_) {
                setState(() => _showVolume = false);
              },
              child: Container(
                width: 60,
                height: 250,
                child: Card(
                  color: widget.backgroundColor,
                  shape: RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(100)),
                  child: RotatedBox(
                    quarterTurns: -1,
                    child: SliderTheme(
                      data: SliderThemeData(
                        activeTrackColor: widget.activeColor,
                        inactiveTrackColor: widget.inactiveColor,
                        thumbColor: widget.thumbColor,
                      ),
                      child: Slider(
                        min: 0.0,
                        max: 1.0,
                        value: players[widget.playerId]!.general.volume,
                        onChanged: (volume) {
                          players[widget.playerId]!.setVolume(volume);
                          setState(() {});
                        },
                      ),
                    ),
                  ),
                ),
              ),
            ),
          ),
        ),
        MouseRegion(
          onEnter: (_) {
            setState(() => _showVolume = true);
          },
          onExit: (_) {
            setState(() => _showVolume = false);
          },
          child: IconButton(
            color: Colors.white,
            onPressed: () => muteUnmute(),
            icon: Icon(getIcon()),
          ),
        ),
      ],
    );
  }

  IconData getIcon() {
    if (players[widget.playerId]!.general.volume > .5)
      return Icons.volume_up_sharp;
    else if (players[widget.playerId]!.general.volume > 0)
      return Icons.volume_down_sharp;
    else
      return Icons.volume_off_sharp;
  }

  void muteUnmute() {
    if (players[widget.playerId]!.general.volume > 0) {
      unmutedVolume = players[widget.playerId]!.general.volume;
      players[widget.playerId]!.setVolume(0);
    } else {
      players[widget.playerId]!.setVolume(unmutedVolume);
    }
    setState(() {});
  }
}
