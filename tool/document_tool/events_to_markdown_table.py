import io
import yaml
from markdownTable import markdownTable 

with open('./src/event/pos_event.yaml', 'r') as stream:
    try:
        events = yaml.safe_load(stream)
    except yaml.YAMLError as exc:
        print(exc)

table = markdownTable(events["Root"]["Event"]).setParams(row_sep = 'markdown', quote = False).getMarkdown()

event_markdown_file = open("./doc/troubleshooting/events.md", "w")
event_markdown_file.write(table)
event_markdown_file.close()
