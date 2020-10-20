from flask import Flask, request
from flask_mail import Mail, Message

app = Flask(__name__)
mail= Mail(app)

app.config['MAIL_SERVER']='smtp.gmail.com'
app.config['MAIL_PORT'] = 465
app.config['MAIL_USERNAME'] = 'projetodetisa@gmail.com'
app.config['MAIL_PASSWORD'] = 'tisa012345678'
app.config['MAIL_USE_TLS'] = False
app.config['MAIL_USE_SSL'] = True
mail = Mail(app)

@app.route("/notificar")
def index():
  email = request.args.get('email')
  msg = Message('NOTIFICAÇÃO - Alteração de Temperatura', sender = 'projetodetisa@gmail.com', recipients = [email])
  msg.body = "A temperatura atual é de " + str(request.args.get('temperatura'))
  mail.send(msg)
  return "Sent"

if __name__ == '__main__':
  print("test")
  app.run(debug=True, port=80, host='172.18.1.4')