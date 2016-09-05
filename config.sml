#############################################################################################################
#
#  This is the NanoboyAdvance configuration file.
#  Do not touch it, if you don't know what you do. After all you can edit this file via the GUI.
#
#  This is a custom, tree-based markup language I developed, because I distaste XML, YAML, INI and JSON
#  (the truth is I was just too lazy and needed something that works and is easily parsable but shhh..)
#  So invented this super cool new standard which I call Shitty Markup Language (SML)
#  and is yet another "standard" (https://xkcd.com/927/).
#
#  To anyone, desperately trying to work with this: I'M SORRY!
#  However I have prepared a small reference:
#  ~name begins a new root-node with the name "name".
#  +name begins a new sub-node with the name "name".
#  - moves one node upwards/back.
#  key: value sets key to value.
#  # is a comment, as you may have noticed by now :P
#
#  If you insane enough, feel free to use this format everywhere you want. It is public domain essentially.
#
#  -- Frederic Meyer (flerovium^-^)
#
#############################################################################################################

~Audio
    Enabled: true

    # Selects the audio driver (0 = None, 1 = SDL2)
    Driver: 1

    +Quality
        SampleRate: 44100
        BufferSize: 1024
