#Voor alle imports kijk naar de volgende links
#https://pythonprogramming.net/using-pip-install-for-python-modules/ kijk naar filmpje voor matplotlib
#https://pypi.python.org/pypi/Pillow/4.3.0
#https://pypi.python.org/pypi/pyserial/2.7


from tkinter import *
import tkinter as tk
from tkinter import ttk
from PIL import ImageTk, Image
import matplotlib
import serial

matplotlib.use("TkAgg")

from matplotlib.backends.backend_tkagg import  FigureCanvasTkAgg, NavigationToolbar2TkAgg
from matplotlib.figure import Figure



#Variabelen die gebruikt worden in de code
temp_bovengrens = 20
standaard_bovengrensT = 20
temp_ondergrens = 10
standaard_ondergrensT = 10
licht_bovengrens = 900
standaard_bovengrensL = 900
licht_ondergrens = 600
standaard_ondergrensL = 600
inrolstand = 2
standaard_inrol = 2
uitrolstand = 10
standaard_uitrol = 10


#Aanzetten connectie tussen python en arduino
# SerialArduino = serial.Serial('COM4', 9600)


#Maken de class voor de GUI
class Window(tk.Tk):
    def __init__(self,):
        tk.Tk.__init__(self,)

        #status
        self.statusLabel = Label(self, text= "status rolluik: niet bekend", font=5)
        self.statusLabel.place(x= 0, y= 0)

        #in- en uitrol buttons
        self.handmatigLabel = Label(self, text= "handmatige bediening", font=15)
        self.handmatigLabel.place(x=790, y=120)
        self.uitrolLabel = Label(self, text= "Open de rolluiken: ")
        self.uitrolLabel.place(x=725, y=150)
        self.uitrolButton = Button(self, text= "Openen", command=self.openLuik, height=1, width=9)
        self.uitrolButton.place(x=830, y=150)
        self.inrolLabel = Label(self, text="Sluit de rolluiken: ")
        self.inrolLabel.place(x=730, y=200)
        self.inrolButton = Button(self, text= "Sluiten",command=self.sluitLuik, height=1, width=9)
        self.inrolButton.place(x=830, y=200)

        #rolstand
        self.rolStandLabel = Label(self, text= "maximum in- en uitrolstand", font=15)
        self.rolStandLabel.place(x=770, y=280)

        self.maxInrolLabel = Label(self, text= "inrolstand (CM): ")
        self.maxInrolLabel.place(x=705, y=320)
        self.inrolWaardeEntry = Entry(self)
        self.inrolWaardeEntry.insert(0, inrolstand)
        self.inrolWaardeEntry.place(x=795, y=320, width=80)
        self.maxInrolButton = Button(self, text= "zet", command=self.set_inrol)
        self.maxInrolButton.place(x=880, y=318)

        self.maxUitrolLabel = Label(self, text= "Uitrolstand (CM): ")
        self.maxUitrolLabel.place(x=700, y=360)
        self.uitrolWaardeEntry = Entry(self)
        self.uitrolWaardeEntry.insert(0, uitrolstand)
        self.uitrolWaardeEntry.place(x=795, y=360, width=80)
        self.maxUitrolButton = Button(self, text= "zet", command=self.set_uitrol)
        self.maxUitrolButton.place(x=880, y=358)

        #Temperatuurwaarde entries
        self.tempLabel = Label(self, text="Waarde instellingen temperatuur", font=15)
        self.tempLabel.place(x=60, y= 655)

        self.temp_onderWaardeEntry = Entry(self)
        self.temp_onderWaardeEntry.insert(0, temp_ondergrens)
        self.temp_onderWaardeEntry.place(x=100, y=680, width=80)
        self.temp_onderWaardeButton = Button(self, text= "zet", command=self.set_tempOnder)
        self.temp_onderWaardeButton.place(x=200, y=680)
        self.temp_onderWaardeLabel = Label(self, text="Onderwaarde:")
        self.temp_onderWaardeLabel.place(x=16, y=680)

        self.temp_bovenWaardeEntry = Entry(self)
        self.temp_bovenWaardeEntry.insert(0, temp_bovengrens)
        self.temp_bovenWaardeEntry.place(x=100, y=720, width=80)
        self.temp_bovenWaardeButton = Button(self, text= "zet", command=self.set_tempBoven)
        self.temp_bovenWaardeButton.place(x=200, y= 720)
        self.temp_bovenWaardeLabel = Label(self, text="Bovenwaarde:")
        self.temp_bovenWaardeLabel.place(x=16, y= 718)

        #Lichtsensorwaarde entries
        self.lichtLabel = Label(self, text="Waarde instellingen lichtsensor", font=15)
        self.lichtLabel.place(x=460, y= 655)

        self.licht_onderWaardeEntry = Entry(self)
        self.licht_onderWaardeEntry.insert(0, licht_ondergrens)
        self.licht_onderWaardeEntry.place(x=500, y=680, width=80)
        self.licht_onderWaardeButton = Button(self, text= "zet", command=self.set_lichtOnder)
        self.licht_onderWaardeButton.place(x=600, y=680)
        self.licht_onderWaardeLabel = Label(self, text="Onderwaarde:")
        self.licht_onderWaardeLabel.place(x=416, y=680)

        self.licht_bovenWaardeEntry = Entry(self)
        self.licht_bovenWaardeEntry.insert(0, licht_bovengrens)
        self.licht_bovenWaardeEntry.place(x=500, y=720, width=80)
        self.licht_bovenWaardeButton = Button(self, text= "zet", command=self.set_lichtBoven)
        self.licht_bovenWaardeButton.place(x=600, y= 720)
        self.licht_bovenWaardeLabel = Label(self, text="Bovenwaarde:")
        self.licht_bovenWaardeLabel.place(x=416, y= 718)
        #Img display
        #load = Image.open("rolluiken.jpg")
        #render = ImageTk.PhotoImage(load)

        #img = Label(self, image=render)
        #img.image = render
        #img.place(x=790, y=10)

        f = Figure(figsize=(7, 5), dpi=100)
        a = f.add_subplot(111)
        a.plot([1,2,3,4,5,6,7,8],[5,2,1,5,6,7,8,9])

        canvas = FigureCanvasTkAgg(f, self)
        canvas.show()
        canvas.get_tk_widget().pack(side=tk.LEFT, expand=False)

    # Hieronder staan de button functies
    def set_lichtBoven(self):
        global licht_bovengrens
        if len(self.licht_bovenWaardeEntry.get()) != 0:
            bovengrens = int(self.licht_bovenWaardeEntry.get())
            if bovengrens > licht_ondergrens:
                licht_bovengrens = bovengrens
                bovengrens_send = ("K" + str(bovengrens))
                print(bovengrens_send)
            else:
                self.licht_bovenWaardeEntry.delete(0, tk.END)
                self.licht_bovenWaardeEntry.insert(tk.INSERT, standaard_bovengrensL)
                print("REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE")
        else:
            print("Het invoer veld mag niet leeg zijn.")

    def set_lichtOnder(self):
        global licht_ondergrens
        if len(self.licht_onderWaardeEntry.get()) !=0:
            ondergrens = int(self.licht_onderWaardeEntry.get())
            if ondergrens < licht_bovengrens:
                licht_ondergrens = ondergrens
                ondergrens_send = ("u" + str(ondergrens))
                print(ondergrens_send)
            else:
                self.licht_onderWaardeEntry.delete(0, tk.END)
                self.licht_onderWaardeEntry.insert(tk.INSERT, standaard_ondergrensL)
                print("REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE")
        else:
            print("Het invoer veld mag niet leeg zijn.")

    def set_tempBoven(self):
        global temp_bovengrens
        if len(self.temp_bovenWaardeEntry.get()) != 0:
            bovengrens = int(self.temp_bovenWaardeEntry.get())
            if bovengrens > temp_bovengrens:
                temp_bovengrens = bovengrens
                bovengrens_send = ("!" + str(bovengrens))
                print(bovengrens_send)
            else:
                self.temp_bovenWaardeEntry.delete(0, tk.END)
                self.temp_bovenWaardeEntry.insert(tk.INSERT, standaard_bovengrensT)
                print("REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE")
        else:
            print("Het invoer veld mag niet leeg zijn.")

    def set_tempOnder(self):
        global temp_ondergrens
        if len(self.temp_onderWaardeEntry.get()) != 0:
            ondergrens = int(self.temp_onderWaardeEntry.get())
            if ondergrens < temp_ondergrens:
                temp_ondergrens = ondergrens
                ondergrens_send = ("q" + str(ondergrens))
                print(ondergrens_send)
            else:
                self.temp_onderWaardeEntry.delete(0, tk.END)
                self.temp_onderWaardeEntry.insert(tk.INSERT, standaard_ondergrensT)
                print("REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE")
        else:
            print("Het invoer veld mag niet leeg zijn.")

    def set_inrol(self):
        global inrolstand
        if len(self.inrolWaardeEntry.get()) != 0:
            inrol = int(self.inrolWaardeEntry.get())
            if inrol < uitrolstand:
                inrolstand = inrol
                inrolstand_send = ("o" + str(inrol))
                print(inrolstand_send)
            else:
                self.inrolWaardeEntry.delete(0, tk.END)
                self.inrolWaardeEntry.insert(tk.INSERT, standaard_inrol)
                print("REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE")
        else:
            print("Het invoer veld mag niet leeg zijn.")

    def set_uitrol(self):
        global uitrolstand
        if len(self.uitrolWaardeEntry.get()) != 0:
            uitrol = int(self.uitrolWaardeEntry.get())
            if uitrol > inrolstand:
                uitrolstand = uitrol
                uirolstand_send = ("p" + str(uitrol))
                print(uirolstand_send)
            else:
                self.uitrolWaardeEntry.delete(0, tk.END)
                self.uitrolWaardeEntry.insert(tk.INSERT, standaard_uitrol)
                print("REEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE")
        else:
            print("Het invoer veld mag niet leeg zijn.")

    def openLuik(self):
        #functie toevoegen voor het opnenen van de rolluiken
        self.statusLabel.config(text="status rolluik: is open")

    def sluitLuik(self):
        #funcite toevoegen voor het verwijderen van de rolluiken
        self.statusLabel.config(text="status rolluik: is gesloten")


root = Window()
root.title("Centrale")
root.geometry("1000x800")
root.mainloop()

