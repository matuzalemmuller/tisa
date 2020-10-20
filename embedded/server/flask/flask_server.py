import os
from flask import Flask, request
from flask_mail import Mail, Message

app = Flask(__name__)
mail= Mail(app)

app.config['MAIL_SERVER']='smtp.gmail.com'
app.config['MAIL_PORT'] = 465
app.config['MAIL_USERNAME'] = os.environ.get('APP_EMAIL_ADDRESS')
app.config['MAIL_PASSWORD'] = os.environ.get('APP_PASSWORD')
app.config['MAIL_USE_TLS'] = False
app.config['MAIL_USE_SSL'] = True
mail = Mail(app)

@app.route("/notificar")
def index():
  email = str(request.args.get('email'))
  temperatura = str(request.args.get('temperatura'))
  msg = Message('NOTIFICAÇÃO - Alteração de Temperatura', sender = 'projetodetisa@gmail.com', recipients = [email])
  msg.body = "A temperatura atual é de " + temperatura
  mail.send(msg)
  print("\n\n\n\n\n" + email + "   " + temperatura)
  return "Sent"

if __name__ == '__main__':
  app.run(debug=True, port=80, host='172.18.1.3')