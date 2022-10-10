import base64, zlib, zmq, random
from os import system

objectKeys = {1:"Object ID",2:"X Position",3:"Y Position",4:"Flipped Horizontally",5:"Flipped Vertically",6:"Rotation",7:"Red",8:"Green",9:"Blue",10:"Duration",11:"Touch Triggered",12:"Secret Coin ID",13:"Special Object Checked",14:"Tint Ground",15:"Player Color 1",16:"Player Color 2",17:"Blending",20:"Editor Layer 1",21:"Main Color Channel ID",22:"Secondary Color Channel ID",23:"Target Color ID",24:"Z Layer",25:"Z Order",28:"Offset X",29:"Offset Y",30:"Easing",31:"Text",32:"Scaling",34:"Group Parent",35:"Opacity",41:"Main Color HSV Enabled",42:"Secondary Color HSV Enabled",43:"Main Color HSV",44:"Secondary Color HSV",45:"Fade In",46:"Hold",47:"Fade Out",48:"Pulse Mode",49:"Copied Color HSV",50:"Copied Color ID",51:"Target Group ID",52:"Pulse Target Type",54:"Yellow Teleportation Portal Y Offset",55:"Teleport Portal Ease",56:"Activate Group",57:"Group IDs",58:"Lock To Player X",59:"Lock To Player Y",60:"Copy Opacity",61:"Editor Layer 2",62:"Spawn Triggered",63:"Spawn Delay",64:"Don't Fade",65:"Main Only",66:"Detail Only",67:"Don't Enter",68:"Degrees",69:"Times 360",70:"Lock Object Rotation",71:"Secondary Group ID",72:"X Mod",73:"Y Mod",75:"Strength",76:"Animation ID",77:"Count",78:"Subtract Count",79:"Pickup Mode",80:"Item/Block ID",81:"Hold Mode",82:"Toggle Mode",84:"Interval",85:"Easing Rate",86:"Exclusive",87:"Multi-Trigger",88:"Comparison",89:"Dual Mode",90:"Speed",91:"Follow Delay",92:"Y Offset",93:"Trigger On Exit",94:"Dynamic Block",95:"Block B ID",96:"Disable Glow",97:"Custom Rotation Speed",98:"Disable Rotation",99:"Multi Activate (Orbs)",100:"Enable Use Target",101:"Target Pos Coordinates",102:"Editor Disable",103:"High Detail",104:"Multi Activate (Triggers)",105:"Max Speed",106:"Randomize Start",107:"Animation Speed",108:"Linked Group ID", 36:"Unknown1", 74:"Unknown2"}

def decode_level(level_data: str) -> str:
    #if is_official_level:
     #   level_data = 'H4sIAAAAAAAAA' + level_data
    base64_decoded = base64.urlsafe_b64decode(level_data.encode())
    # window_bits = 15 | 32 will autodetect gzip or not
    decompressed = zlib.decompress(base64_decoded, 15 | 32)
    return decompressed.decode()

def build_level(level_data: str, cell_size: int) -> dict:
    level = {}
    for object in level_data.split(';')[1:]:
        object = object.split(',')
        if len(object) < 6:
            continue

        x = int(object[3])
        y = int(object[5])
        id = int(object[1])

        x= round(x/cell_size)
        y= round(y/cell_size)
        level[(x,y)] = id

    return level


cell_size = 20
if __name__ == "__main__":
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind("tcp://*:6969")
    print("Server initialized...")

    data = ""
    lvl = {}
    while True:
        #  Wait for next request from client
        message = socket.recv()
        print(f"Received request: {message}")
        message = message.decode('UTF-8')

        args = message.split(":")

        #Set level data
        if str(args[0]) == "init":
            data = decode_level(args[1])
            lvl = build_level(data, cell_size)

        #Visualization
        if str(args[0]) == "update":
            pos_x, pos_y = tuple(map(float, args[1].replace("'","").split(",")))

            pos_x = round(pos_x/cell_size)
            pos_y = round(pos_y/cell_size)
            w, h = (20, 10)
            system("cls")
            for y in range(h, -h, -1):
                for x in range(-w, w):
                    if (pos_x + x, pos_y + y) in lvl:
                        print(lvl[(pos_x + x, pos_y + y)], end = "")
                    else:
                        print(".", end="")
                print("")

        #Send reply back to client (Currently just jumping randomly)
        socket.send_string(str(random.randint(0,30)))