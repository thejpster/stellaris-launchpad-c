#!/usr/bin/python
# -*- coding: utf-8 -*-

import wx
import random
import threading
import binascii

# Define notification event for thread completion
EVT_RESULT_ID = wx.NewId()

class ResultEvent(wx.PyEvent):
    """Simple event to carry arbitrary result data."""
    def __init__(self, data):
        """Init Result Event."""
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_RESULT_ID)
        self.data = data

def process_fifo(lcd):
    def unpack(s):
        return binascii.unhexlify(s)
    def colour_tuple(s):
        col = int(s, base=0)
        r = (col >> 16) & 0xFF
        g = (col >> 8) & 0xFF
        b = col & 0xFF
        return (r,g,b)
    while True:
        f = open("lcd_fifo", "r", buffering=False)
        print "Thread started"
        while True:
            # This seems to be the least latency way of reading from the FIFO
            msg = f.readline()
            if not msg:
                # No result = EOF
                break
            msg = msg.strip()
            if msg.startswith("reset"):
                wx.PostEvent(lcd, ResultEvent(("clear",)))
            if msg.startswith("box"):
                try:
                    (cmd, x1, x2, y1, y2, col) = msg.split(" ")
                except ValueError:
                    raise
                x1 = int(x1, base=0)
                y1 = int(y1, base=0)
                x2 = int(x2, base=0)
                y2 = int(y2, base=0)
                data = ("setBox", (x1,y1), (x2,y2), colour_tuple(col))
                wx.PostEvent(lcd, ResultEvent(data))
            if msg.startswith("bitmap"):
                try:
                    (cmd, x1, x2, y1, y2, fg, bg, bitmap) = msg.split(" ")
                except ValueError:
                    raise
                x1 = int(x1, base=0)
                y1 = int(y1, base=0)
                x2 = int(x2, base=0)
                y2 = int(y2, base=0)
                fg = colour_tuple(fg)
                bg = colour_tuple(bg)
                bitmap = unpack(bitmap)
                data = ("setBitmap", (x1,y1), (x2,y2), fg, bg, bitmap)
                wx.PostEvent(lcd, ResultEvent(data))
            elif msg.startswith("plot"):
                try:
                    (cmd, x, y, col) = msg.split(" ")
                except ValueError:
                    raise
                x = int(x, base=0)
                y = int(y, base=0)
                data = ("setPixel", x, y, colour_tuple(col))
                wx.PostEvent(lcd, ResultEvent(data))

class LCD(wx.Frame):
    def __init__(self, parent, title):
        super(LCD, self).__init__(parent, title=title, 
            size=(272, 480))
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        self.Connect(-1, -1, EVT_RESULT_ID, self.OnMessage)
        self.Centre()
        self.Show()
        self.bmp = wx.EmptyBitmap(272, 480)
        self.clearBitmap()
        self.count = 0

    def OnMessage(self, evt):
        f = evt.data[0]
        args = evt.data[1:]
        if f == "clear":
            self.clearBitmap()
        elif f == "setPixel":
            self.setPixel(*args)
        elif f == "setBox":
            self.setBox(*args)
        elif f == "setBitmap":
            self.setBitmap(*args)
        self.Refresh()
        self.Update()

    def clearBitmap(self):
        dc = wx.MemoryDC()
        dc.SelectObject(self.bmp)
        dc.SetBrush(wx.Brush((0,0,0), wx.SOLID))
        dc.SetPen(wx.Pen((0,0,0)))
        dc.DrawRectangle(0,0,272,480)
        dc.SelectObject(wx.NullBitmap)
        #del dc # need to get rid of the MemoryDC before Update() is called.

    def setBox(self, p1, p2, colour):
        dc = wx.MemoryDC()
        dc.SelectObject(self.bmp)
        dc.SetBrush(wx.Brush(colour, wx.SOLID))
        dc.SetPen(wx.Pen(colour))
        (x,y) = p1
        (w,h) = (1 + p2[0] - p1[0], 1 + p2[1] - p1[1])
        dc.DrawRectangle(x, y, w, h)
        dc.SelectObject(wx.NullBitmap)
        #del dc # need to get rid of the MemoryDC before Update() is called.

    def setBitmap(self, p1, p2, fg, bg, data):
        dc = wx.MemoryDC()
        dc.SelectObject(self.bmp)
        fg_pen = wx.Pen(fg)
        bg_pen = wx.Pen(bg)
        x,y = p1
        min_x,min_y = p1
        max_x,max_y = p2
        for b in data:
            byte = ord(b)
            for i in range(8):
                is_fg = byte & 0x80
                byte <<= 1
                if is_fg:
                    dc.SetPen(fg_pen)
                else:
                    dc.SetPen(bg_pen)
                dc.DrawPoint(x,y)
                if (x == max_x):
                    x = min_x
                    y += 1
                else:
                    x += 1
        dc.SelectObject(wx.NullBitmap)
        #del dc # need to get rid of the MemoryDC before Update() is called.

    def setPixel(self, x, y, colour):
        dc = wx.MemoryDC()
        dc.SelectObject(self.bmp)
        dc.SetPen(wx.Pen(colour))
        dc.DrawPoint(x, y)
        del dc # need to get rid of the MemoryDC before Update() is called.

    def OnPaint(self, e):
        dc = wx.PaintDC(self)
        dc.DrawBitmap(self.bmp, 0, 0)

if __name__ == '__main__':
    app = wx.App()
    lcd = LCD(None, 'LCD')
    t = threading.Thread(target=process_fifo, args=(lcd,))
    t.daemon = True
    t.start()
    app.MainLoop()
