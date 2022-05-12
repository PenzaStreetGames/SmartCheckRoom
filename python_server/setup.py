from setuptools import setup


name = 'Smart Check Room'
version = '0.0.1'
release = '0.0.1'
setup(
    name='SmartCheckRoom',
    version='0.0.1',
    packages=['server',
              'server/mqtt_handlers'],
    install_requires=["paho-mqtt==1.6.1",
                      "SQLAlchemy==1.4.27",
                      "sphinx==4.5.0",
                      "sphinx_rtd_theme==1.0.0"
                      ],
)
