import matplotlib.pyplot as plt
import pyformulas as pf
import numpy as np

width = 640
height = 480

canvas = np.zeros((height, width))
screen = pf.screen(canvas, 'Testing')

frequency = 250.0
amplitude = 0.1
dim = 1
cur = 0

# NOTE: Make sure audio input isn't muted!
while next():
    nxt = cur + len(outputs[0])
    x = np.arange(cur, nxt, 1)
    y = dim * amplitude * np.sin(2 * np.pi * frequency * x / sample_rate)
    for output in outputs:
        np.copyto(output, y)
    cur = nxt
    dim *= 0.99

    fig = plt.figure()
    dpi = fig.get_dpi()
    fig.set_size_inches(width/dpi, height/dpi)
    plt.axis([0, len(y), -1, 1])
    plt.legend({ 'x', 'y' })
    t = np.arange(len(y))
    plt.plot(t, y)
    fig.canvas.draw()
    image = np.fromstring(fig.canvas.tostring_rgb(), dtype=np.uint8, sep='')
    image = image.reshape(fig.canvas.get_width_height()[::-1] + (3,))
    screen.update(image)
