# Web_server With the ability to accept multiple clients

#==========================================================================================
import logging
import threading
import time
import socket
import matplotlib.pyplot as plt
import numpy as np
import os
import xlsxwriter
import bs4 as bs

#declaring global variables
line1 = []
thread = 0
total_data_dict = {}
current_data_dict = {}
PatientID = 0

#==========================================================================================
#function for serving patient updated data
def patient_data(file):
    global total_data_dict, current_data_dict
    pt_sause = open('patientr/'+file).read()
    pt_soup =  bs.BeautifulSoup(pt_sause, 'html.parser')
    ID = file.split(".")[0]
    if ID in current_data_dict.keys():
        data_list = current_data_dict[ID]
        print(data_list)
    else:
        data_list = ['nodata', 'nodata', 'nodata']

    for i, row in enumerate(pt_soup.find_all('tr')):
        if i>0:
            col = row.find_all('td')
            col[1].clear()
            col[1].append(data_list[i-1])
            # print(col)
    pt_file = str(pt_soup)
    pt_file = "/\nHTTP/1.1 200 OK\n\n"+pt_file
    return pt_file

#==========================================================================================
# function for serving web_page

def web_page():
    http_response = open('patientr.html').read()
    http_response = "/\nHTTP/1.1 200 OK\n\n"+http_response
    return http_response


#==========================================================================================
# function for serving namechange

def namechange(folder):
    sause = open('patientr.html').read()
    soup = bs.BeautifulSoup(sause, 'html.parser')
    dev_id = folder[2]
    i = int(dev_id.split("_")[1])
    button = soup.find_all('button')
    button[i-1].clear()
    button[i-1].append(folder[3])
    with open("patientr.html", "w", encoding = 'utf-8') as file:
        # prettify the soup object and convert it into a string  
        file.write(str(soup.prettify()))


#==========================================================================================

#function for Serving the client

def Client_thread(Client, address):
    print(threading.currentThread().getName(), 'Starting')
    global total_data_dict, current_data_dict, thread, PatientID
    request_data = Client.recv(1024)
    request_data = request_data.decode('utf-8')
    # print(request_data)
    code = request_data.split(" ", 2)

    if code[0] == "client" :
        PatientID = address
        device_ID = code[1]
        datalog = [['SPO2', 'heart Rate', 'Temperature']]



        # Create a workbook and add a worksheet.
        workbook = xlsxwriter.Workbook(str(address)+".xlsx")
        worksheet = workbook.add_worksheet()

        # Start from the first cell. Rows and columns are zero indexed.
        row = 0
        col = 0

        for c1, c2, c3 in datalog:
            worksheet.write(row, col,     c1)
            worksheet.write(row, col + 1, c2)
            worksheet.write(row, col + 2, c3)
            
            row = row+1

        j = 0
        while j<200:
            data = Client.recv(2048)
            data = data.decode('utf-8')
            data_list = data.split(",")
            # data_list = [float(i) for i in data_list]
            current_data_dict[device_ID] = data_list

            print(data_list, address, device_ID)
            if ('exit' in data):
                print("exit krra")
                Client.close()
                break
            datalog.append(data_list)
            # Iterate over the data and write it out row by row.
            j = j+1

        for c1, c2, c3 in datalog:
            worksheet.write(row, col,     c1)
            worksheet.write(row, col + 1, c2)
            worksheet.write(row, col + 2, c3)
                
            row += 1

        workbook.close()
        total_data_dict[device_ID] = datalog
        Client.close()

    else:
        if code[1] == '/':
            Client.sendall(bytes(web_page(), "utf-8"))
            Client.close()

        else:
            folder = code[1].split("/", 3)
            # print(folder)
            if folder[1] == 'patientr':
                file = folder[2]
                Client.sendall(bytes(patient_data(file), "utf-8"))
                Client.close()
            elif folder[1] == 'namechange':
                namechange(folder)
                Client.close()
            else:
                Client.close()


    thread = thread-1
    print(threading.currentThread().getName(), 'Exiting')


#============================================================================================


#function for accepting clients

def Client_accept(ServerSocket):
    global thread
    print(threading.currentThread().getName(), 'Starting')
    while True:
        Client, address = ServerSocket.accept()
        print('Connected to: ' + address[0] + ':' + str(address[1]))
        # type(address[0]) = str, type(address[1]) = int
        client_thread = threading.Thread(name=address[0] + ':' + str(address[1]), target  = Client_thread, args = (Client, address, ))    #Separating the thread for serving
        thread = thread+1
        client_thread.start()                                                                                   #the client
    print(threading.currentThread().getName(), 'Exiting')

 #======================================================================================

#main function (Main Thread)

if __name__ == "__main__":
    ServerSocket = socket.socket()                                      #setting up the socket
    host = "192.168.104.34"#socket.gethostname()                                                 #
    port = 4000                                                                 #
    ThreadCount = 0                                                             #
    ServerSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    ServerSocket.bind((host, port))                                             #

    print('Waitiing for a Connection..')                                        #
    print("hostname " + socket.gethostbyname(host), port, " global: 103.37.200.133")                       #
    ServerSocket.listen(5)                                                      #
    print("Web server Started")                                         #server started
    acceptin_client = threading.Thread(name='accepting_client', target  = Client_accept, args = (ServerSocket, ))      #Making a Thread For accepting client
    acceptin_client.start()                                                                                         #starting the thread
