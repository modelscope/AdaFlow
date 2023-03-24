#setup.py
from setuptools import setup

setup(name='cli',
      version='0.0.1',
      description='AdaFlow is a multimedia pipeline frameworks to offer Deep Learning model integration.',
      url='https://github.com/modelscope/adaflow',
      author="long.qul, jingyao.ww, jiayong.djy, hanbing.han, yulin.yu@alibaba-inc.com",
      author_email="long.qul@alibaba-inc.com, jingyao.ww@alibaba-inc.com, hanbing.han@alibaba-inc.com, yulin.yu@alibaba-inc.com",
      license='MIT License',
      packages=['cli'],
      install_requires=['requests'],
      entry_points={
          'console_scripts': ['adaflow=cli.cli:run_cmd'],

      },
      zip_safe=False)