import './globals.css'

export const metadata = {
  title: 'Thun - OCR Project of Duch Panhathun | Creative Projects and Works',
  description: 'Welcome to our webstie this website is a project for OCR. You can upload image and it will be converted to word or document etc.',
  keywords: 'Thun, Duch Panhathun, duchpanhathun, OCR Project, OCR',
  openGraph: {
    title: 'Thun - OCR Project of Duch Panhathun | Creative Projects and Works',
    description: 'Welcome to our webstie this website is a project for OCR. You can upload image and it will be converted to word or document etc.',
    url: 'https://thun-ocr-project.app',
    siteName: 'Duch Panhathun OCR Project',
    images: [
      {
        url: 'https://media.licdn.com/dms/image/D5603AQGJV9inu2faiA/profile-displayphoto-shrink_400_400/0/1718933205181?e=1724284800&v=beta&t=r20uvTQM2rCfP5RjmTJHmRFpsnxfJh4hf0lOCocmXBk',
        width: 400,
        height: 400,
      },
    ],
    locale: 'en_US',
    type: 'website',
  },
  twitter: {
    card: 'summary_large_image',
    title: 'Thun - OCR Project | Creative Projects and Works',
    description: 'Welcome to our webstie this website is a project for OCR. You can upload image and it will be converted to word or document etc.',
    images: ['https://media.licdn.com/dms/image/D5603AQGJV9inu2faiA/profile-displayphoto-shrink_400_400/0/1718933205181?e=1724284800&v=beta&t=r20uvTQM2rCfP5RjmTJHmRFpsnxfJh4hf0lOCocmXBk'],
  },
}

export default function RootLayout({ children }) {
  return (
    <html lang="en">
      <head>
        <link rel="icon" type="image/x-icon" href="/public/img/fav_icon.ico" />
        <link href='https://unpkg.com/boxicons@2.1.4/css/boxicons.min.css' rel='stylesheet' />
        <link rel="stylesheet" href="/index.css" />
      </head>
      <body>
        <div id="root">
          {children}
        </div>
      </body>
    </html>
  )
}