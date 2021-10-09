import time
import sys

image = "demo.jpg"
# out_image = "test_out_cat.jpg"
out_image_2 = "myDemo.jpg"
data = []
with open(image,"rb") as f:
    data = f.read()
    # print(data)

temp = 0
last_temp = 0
header = False 
new_image = []
for byte in data:
    temp_last = temp 
    temp = byte 
    print(byte)

    if(header): #if header found 

        #look for end of image 
        if (temp == 0xD9) and (temp_last == 0xFF):
            header = False 
            new_image.append(temp) #end of image
            print("found end") 
            break #we'll just break for now, actual application wll do something more sophisticated

        else:
            new_image.append(temp)

    if (temp == 0xD8) and (temp_last==0xFF):
        header = True
        print("found start")
        new_image.append(temp_last)
        new_image.append(temp)
           

# #original image 
# with open(out_image,"wb") as f:
#     f.write(data)

with open(out_image_2, "wb") as f:
    new_image = bytearray(new_image)
    # print(new_image)
    f.write(new_image)

