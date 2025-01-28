project('Limited TNiC', 'c')

arguments = []
if get_option('buildtype') == 'debug'
  arguments += ['-DDEBUG -g3']
endif

exec = executable(
  'tnic',
  ['src/main.c', 'src/database.c', 'src/playlist.c', 'src/music.c', 'src/embeds.c', 'src/map/map.c'],
  c_args: arguments,
  link_args: [ '-lcoglink', '-pthread', '-ldiscord', '-lcurl' ]
)