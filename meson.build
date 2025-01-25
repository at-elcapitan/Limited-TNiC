project('Limited TNiC', 'c')

exec = executable(
  'tnic',
  ['src/main.c', 'src/database.c', 'src/playlist.c', 'src/music.c'],
  c_args: [],
  link_args: [ '-lcoglink', '-pthread', '-ldiscord', '-lcurl' ]
)