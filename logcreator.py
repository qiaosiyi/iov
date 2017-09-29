# -*- encoding:utf-8 -*-
import logging

def createLogger(logger_name, log_path, log_level):
	# create logger
	logger = logging.getLogger(logger_name)
	logger.setLevel(log_level)
	
	# create file handler
	fh = logging.FileHandler(log_path)
	fh.setLevel(log_level)
	
	# create formatter
	fmt = "[%(asctime)-15s] [%(levelname)s] [FILE \"%(pathname)s\", in %(funcName)s,  line %(lineno)d]\n\t%(message)s"
	datefmt = "%a %d %b %Y %H:%M:%S"
	formatter = logging.Formatter(fmt, datefmt)
	
	# add handler and formatter to logger
	fh.setFormatter(formatter)
	logger.addHandler(fh)	
	return logger

