import matplotlib.pyplot as plt
import numpy as np

if __name__ == '__main__':
    np.random.seed(65342)
    scale = 50
    array = np.random.randint(5, size=(3, 6))
    array = np.kron(array, np.ones((scale, scale), dtype=array.dtype))
    plt.imsave('logo.png', array, cmap='YlGn')
