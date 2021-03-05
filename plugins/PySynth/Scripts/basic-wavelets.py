import matplotlib.pyplot as plt
import pyformulas as pf
import numpy as np

import pywt
import pywt.data

width = 640
height = 480

canvas = np.zeros((height, width))
screen = pf.screen(canvas, 'Testing')

# NOTE: Make sure audio input isn't muted!
while next():
    (cA, cD) = pywt.dwt(inputs[0], 'db2', 'smooth')
    result = pywt.idwt(cA, cD, 'db2', 'smooth')
    #(cA, cD) = pywt.dwt(inputs[0], 'db2', 'periodization')
    #result = pywt.idwt(cA, cD, 'db2', 'periodization')
    for output in outputs:
        np.copyto(output, result)

    fig = plt.figure()
    dpi = fig.get_dpi()
    fig.set_size_inches(width/dpi, height/dpi)
    plt.axis([0, len(cA), -1, 1])
    plt.legend({ 'cA', 'cD' })
    t = np.arange(len(cA))
    plt.plot(t, cA, cD)
    fig.canvas.draw()
    image = np.fromstring(fig.canvas.tostring_rgb(), dtype=np.uint8, sep='')
    image = image.reshape(fig.canvas.get_width_height()[::-1] + (3,))
    screen.update(image)
