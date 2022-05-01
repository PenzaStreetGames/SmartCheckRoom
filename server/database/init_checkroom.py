from json import loads, dumps
from database import *


def init_checkroom():
    config_file = "checkroom_struct.json"

    with open(config_file, "r", encoding="utf-8") as infile:
        struct = loads(infile.read())
    print(struct)

    if not struct["init"]:

        for control_box_info in struct["control_boxes"]:
            control_id = control_box_info["id"]
            nfc_info = control_box_info["nfc"]
            hangers_info = control_box_info["hangers"]
            control_box = ControlBoxService.create(control_id)

            nfc_id = nfc_info["id"]
            nfc = NfcService.create(id=nfc_id, control_id=control_id)
            nfc.control_box = control_box

            for hanger_info in hangers_info:
                hanger_id = hanger_info["id"]

                hanger = HangerService.create(id=hanger_id, control_id=control_id)
                hanger.control_box = control_box

                hooks_info = hanger_info["hooks"]
                hooks_from = hooks_info["from"]
                hooks_to = hooks_info["to"]
                for i in range(hooks_from, hooks_to + 1):
                    hook = HookService.create(id=i, hanger_id=hanger_id)
                    hook.hanger = hanger

        session.commit()
        struct["init"] = True

    with open(config_file, "w", encoding="utf-8") as outfile:
        outfile.write(dumps(struct, indent=2))


if __name__ == '__main__':
    init_checkroom()
