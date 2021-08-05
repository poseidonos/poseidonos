import scenario
import lib

config_data = lib.parser.ArgParser()
scenario.player.play(config_data.GetConfig())
