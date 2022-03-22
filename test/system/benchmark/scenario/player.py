import os
import json
import importlib
import lib
import sys


def play(json_cfg_file):
    print("\n --- [benchmark start] --- ")
    print("open json cfg file: " + json_cfg_file)

    try:
        with open(json_cfg_file, "r") as f:
            config = json.load(f)
    except IOError:
        lib.printer.red(f"{__name__} [IOError] No such file or directory")
        sys.exit(1)
    except json.decoder.JSONDecodeError as e:
        lib.printer.red(f"{__name__} [JSONDecodeError] {e}")
        sys.exit(1)

    for scenario in config["Scenarios"]:
        try:
            module_name = "scenario." + scenario["NAME"]
        except KeyError:
            lib.printer.red(
                f"{__name__} [KeyError] JSON file Scenarios has no KEY 'NAME'")
            sys.exit(1)

        try:
            lib.subproc.sync_run(f"mkdir -p {scenario['OUTPUT_DIR']}")
            lib.subproc.sync_run(f"mkdir -p {scenario['OUTPUT_DIR']}/log")
        except KeyError:
            lib.printer.red(
                f"{__name__} [KeyError] JSON file Scenarios has no KEY 'OUTPUT_DIR'")
            sys.exit(1)
        except Exception as e:
            lib.printer.red(f"{__name__} [Error] {e}")
            sys.exit(1)

        try:
            module = importlib.import_module(module_name)
        except ImportError:
            lib.printer.red(
                f"{__name__} [ImportError] '{module_name}' is not defined")
            sys.exit(1)

        if not hasattr(module, "play"):
            lib.printer.red(
                f"{__name__} [AttributeError] '{module_name}' has no attribute 'play'")
            sys.exit(1)
        if "Targets" not in config:
            lib.printer.red(
                f"{__name__} [KeyError] JSON file has no KEY 'Targets'")
            sys.exit(1)
        if "Initiators" not in config:
            lib.printer.red(
                f"{__name__} [KeyError] JSON file has no KEY 'Initiators'")
            sys.exit(1)
        module.play(config["Targets"], config["Initiators"], scenario)

    print(" --- [benchmark done] --- \n")
    return 0
