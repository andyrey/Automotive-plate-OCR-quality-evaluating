# Automotive-plate-OCR-quality-evaluating
This program works with set of pairs of *.txt files- which are detection&OCR program results versus that of ground truth *_gt.txt files. Each txt file contain readings of all car's plates as string-by-string.Number of strings in each file as many, as plates on the image. Number of file pairs as many, as number of tested images. Detecting&OCR program, working with images, is not here. The order of strings in tested and corresponding GT files may be different- the algorithm will find the best match from the list.
As a matter of the fact, this program uses Edit Distanse algorithm from Dynamic Programming and can compare any letter-digit string pairs,align strings in the pairs, put fines for any deletion,insertion, change or alignment,and evaluate number of errors made by OCR program. If some autoplate was not detected and therefore misses against ground truth, the program will consider this loss in detection score.

Программа Automotive plate OCR quality evaluating предназначена для автоматической оценки
качества чтения плат автомобильных госномеров, работает только с набором текстовых файлов.
Предварительно, должна сработать Программа-0, читающая все автомобильные госномера на снимке и создающая
по одному файлу .txt на каждый одноименный файл .jpg, где существующие номера записаны построчно. 
Программа-оценщик запускается из cmd:
eval_ocr.exe Compare/
где в папке Compare/ содержатся пары текстовых файлов- прочитанные тестируемой
программой (одноименные с соответствующим jpg файлом- ocr файл), и парный ему файл с суффиксом "_gt",
что означает ground truth- то есть истинные. Количество записей-строк в файле  _gt соответствует
количеству госномеров на снимке. Если прочитанных строк меньше, чем в -gt файле, программа фиксирует 
количество неудавшихся детекций.
  Программа подсчитывает все несоответсвия чтения посимвольно, при необходимости выравни-
вает -приводит прочитанную строку с госномером к истинной, при этом штрафуя за каждую вставку
или замену символа одним баллом. Если Программа-0 определила наличие символа, но не смогла его прочитать, 
то ставит символ '?' в прочитанную строку, штраф в этом случае пол-балла.
  Выводом программы является файл report.txt, где приводятся все ошибки по каждому файлу и 
выровненные строки, приведенные к ground truth.
  Итоговый вывод дается как сумма посимвольных ошибок, также количество и % неверно прочитанных
госномеров, и количество  неотдетектированных номеров в ocr-файле против ground truth. 
Данная программа использует алгоритм edit distance.
  ВНИМАНИЕ! Программа прекращает работу, если не все файлы имеют пары: "xx.txt"-> "xx_gt.txt".
Перед запуском программы визуально проверьте парность! 
