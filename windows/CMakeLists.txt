# dart_vlc: A media playback library for Dart & Flutter. Based on libVLC & libVLC++.
#
# Hitesh Kumar Saini
# https://github.com/alexmercerind
# alexmercerind@gmail.com
# GNU Lesser General Public License v2.1
# 

cmake_minimum_required(VERSION 3.15)

set(PROJECT_NAME "dart_vlc")
project(${PROJECT_NAME} LANGUAGES CXX)
set(PLUGIN_NAME "dart_vlc_plugin")

set(LIBVLC_VERSION "3.0.9.2")

include_directories("${PLUGIN_NAME}" INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
add_library("${PLUGIN_NAME}" SHARED
  "dart_vlc_plugin.cpp"
)

apply_standard_settings("${PLUGIN_NAME}")
set_target_properties("${PLUGIN_NAME}" PROPERTIES
  CXX_VISIBILITY_PRESET hidden)
target_compile_definitions("${PLUGIN_NAME}" PRIVATE FLUTTER_PLUGIN_IMPL)
target_include_directories("${PLUGIN_NAME}" INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/../dartvlc"
)

set(LIBVLC_BINARIES "${CMAKE_CURRENT_SOURCE_DIR}/bin")

set(LIBVLC_URL "http://download.videolan.org/pub/videolan/vlc/${LIBVLC_VERSION}/win64/vlc-${LIBVLC_VERSION}-win64.7z")
set(LIBVLCPP_URL "https://github.com/videolan/libvlcpp/archive/master.zip")
set(LIBVLC_ARCHIVE "${LIBVLC_BINARIES}/vlc-${LIBVLC_VERSION}.7z")
set(LIBVLCPP_ARCHIVE "${LIBVLC_BINARIES}/libvlcpp.zip")
set(LIBVLC_SOURCE "${LIBVLC_BINARIES}/vlc-${LIBVLC_VERSION}")
set(LIBVLCPP_SOURCE "${LIBVLC_BINARIES}/libvlcpp-master")


if (NOT EXISTS "${LIBVLC_ARCHIVE}")
  file(DOWNLOAD "${LIBVLC_URL}" "${LIBVLC_ARCHIVE}")
endif()

if (NOT EXISTS "${LIBVLCPP_ARCHIVE}")
  file(DOWNLOAD "${LIBVLCPP_URL}" "${LIBVLCPP_ARCHIVE}")
endif()

set(LIBVLC_ARCHIVE_ROOT "vlc-${LIBVLC_VERSION}/")
set(LIBVLC_PLUGINS
  "access/libfilesystem_plugin.dll"
  "access/libhttp_plugin.dll"
  "access/libhttps_plugin.dll"

  "audio_filter/libsamplerate_plugin.dll"
  "audio_output/libdirectsound_plugin.dll"
  "audio_output/libwaveout_plugin.dll"

  "codec/libavcodec_plugin.dll"
  "codec/libd3d11va_plugin.dll"
  "codec/libdxva2_plugin.dll"
  "codec/libqsv_plugin.dll"

  "demux/libadaptive_plugin.dll"
  "demux/libmkv_plugin.dll"
  "demux/libts_plugin.dll"
  "demux/libogg_plugin.dll"
  "demux/libmp4_plugin.dll"
  "demux/libplaylist_plugin.dll"
  "demux/libh26x_plugin.dll"
  "demux/libavi_plugin.dll"
  "demux/libsubtitle_plugin.dll"
  "demux/libasf_plugin.dll"
  "demux/libflacsys_plugin.dll"
  "demux/libmpc_plugin.dll"
  "demux/libnsc_plugin.dll"
  "demux/libps_plugin.dll"
  "demux/libes_plugin.dll"
  "demux/libreal_plugin.dll"
  "demux/libty_plugin.dll"
  "demux/libimage_plugin.dll"
  "demux/libwav_plugin.dll"
  "demux/libsmf_plugin.dll"
  "demux/libnuv_plugin.dll"
  "demux/libmjpeg_plugin.dll"
  "demux/libpva_plugin.dll"
  "demux/librawvid_plugin.dll"
  "demux/libnsv_plugin.dll"
  "demux/libvoc_plugin.dll"
  "demux/libdemux_stl_plugin.dll"
  "demux/librawdv_plugin.dll"
  "demux/libaiff_plugin.dll"
  "demux/librawaud_plugin.dll"
  "demux/libtta_plugin.dll"
  "demux/libau_plugin.dll"
  "demux/libdemuxdump_plugin.dll"
  "demux/libdiracsys_plugin.dll"
  "demux/libmpgv_plugin.dll"
  "demux/libxa_plugin.dll"
  "demux/libdirectory_demux_plugin.dll"
  "demux/libnoseek_plugin.dll"

  "keystore/libfile_keystore_plugin.dll"
  "keystore/libmemory_keystore_plugin.dll"

  "logger/libconsole_logger_plugin.dll"
  "logger/libfile_logger_plugin.dll"

  "video_chroma/libchain_plugin.dll"
  "video_chroma/libgrey_yuv_plugin.dll"
  "video_chroma/libi420_10_p010_plugin.dll"
  "video_chroma/libi420_nv12_plugin.dll"
  "video_chroma/libi420_rgb_plugin.dll"
  "video_chroma/libi420_rgb_sse2_plugin.dll"
  "video_chroma/libi420_yuy2_plugin.dll"
  "video_chroma/libi420_yuy2_sse2_plugin.dll"
  "video_chroma/libi422_i420_plugin.dll"
  "video_chroma/libi422_yuy2_plugin.dll"
  "video_chroma/libi422_yuy2_sse2_plugin.dll"
  "video_chroma/librv32_plugin.dll"
  "video_chroma/libswscale_plugin.dll"
  "video_chroma/libyuvp_plugin.dll"
  "video_chroma/libyuy2_i420_plugin.dll"
  "video_chroma/libyuy2_i422_plugin.dll"

  "video_output/libvmem_plugin.dll"
)


list(TRANSFORM LIBVLC_PLUGINS PREPEND "plugins/")
set(LIBVLC_FILES libvlc.dll libvlccore.dll sdk ${LIBVLC_PLUGINS})
list(TRANSFORM LIBVLC_FILES PREPEND ${LIBVLC_ARCHIVE_ROOT})

add_custom_target(LIBVLC_EXTRACT ALL)
add_custom_command(
  TARGET LIBVLC_EXTRACT PRE_BUILD
  COMMAND ${CMAKE_COMMAND} -E tar xzf \"${LIBVLC_ARCHIVE}\" -- ${LIBVLC_FILES}
  COMMAND ${CMAKE_COMMAND} -E tar xzf \"${LIBVLCPP_ARCHIVE}\"
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIBVLC_SOURCE}/sdk/include/vlc ${CMAKE_CURRENT_SOURCE_DIR}/include/vlc
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${LIBVLCPP_SOURCE}/vlcpp ${CMAKE_CURRENT_SOURCE_DIR}/include/vlcpp
  WORKING_DIRECTORY "${LIBVLC_BINARIES}"
  DEPENDS "${LIBVLC_ARCHIVE}"
)

target_link_libraries("${PLUGIN_NAME}" PRIVATE
  flutter
  flutter_wrapper_plugin
  "${LIBVLC_SOURCE}/sdk/lib/libvlc.lib"
  "${LIBVLC_SOURCE}/sdk/lib/libvlccore.lib"
)

set(
  dart_vlc_bundled_libraries
  "${LIBVLC_SOURCE}/libvlc.dll"
  "${LIBVLC_SOURCE}/libvlccore.dll"
  "${LIBVLC_SOURCE}/plugins"
  PARENT_SCOPE
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4244 /wd4996")
