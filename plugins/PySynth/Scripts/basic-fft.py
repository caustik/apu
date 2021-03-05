import matplotlib.pyplot as plt
import pyformulas as pf
import numpy as np

from scipy import fftpack
from numpy import fft as fft

width = 640
height = 480

canvas = np.zeros((height, width))
screen = pf.screen(canvas, 'Testing')

# NOTE: Make sure audio input isn't muted!
while next():
    f = fftpack.fft(inputs[0])
    s = fftpack.ifft(f)
    for output in outputs:
        np.copyto(output, s.real)

    fig = plt.figure()
    dpi = fig.get_dpi()
    fig.set_size_inches(width/dpi, height/dpi)
    plt.axis([0, len(f), -1, 1])
    plt.legend({ 'f.real', 'f.imag' })
    t = np.arange(len(f))
    plt.plot(t, f.real, f.imag)
    fig.canvas.draw()
    image = np.fromstring(fig.canvas.tostring_rgb(), dtype=np.uint8, sep='')
    image = image.reshape(fig.canvas.get_width_height()[::-1] + (3,))
    screen.update(image)
