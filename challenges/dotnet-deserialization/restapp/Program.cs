using System;
using System.Collections;
using System.Collections.Generic;
using Nancy;
using Nancy.IO;
using Nancy.Extensions;
using Nancy.Hosting.Self;
using System.Xml;
using System.Xml.Serialization;
using System.IO;

namespace restapp
{
    public class Product
    {
        public string Name;
        public string Description;
        public string ImageURL;
    }

    public class ProductController : NancyModule
    {
        public static Product[] stockProducts = {
            new Product { Name = "Product1", Description = "Sample Product", ImageURL = "NA" },
            new Product { Name = "Product2", Description = "Sample Product", ImageURL = "NA" },
            new Product { Name = "Product3", Description = "Sample Product", ImageURL = "NA" }
        };

        public static List<Product> products = new List<Product>(stockProducts);

        public ProductController() : base("/products") {
            Get("/", _ => 
            {
                XmlRootAttribute root = new XmlRootAttribute("Products");
                XmlSerializer xmlSerializer = new XmlSerializer(typeof(Product[]), root);

                using(StringWriter textWriter = new StringWriter())
                {
                    xmlSerializer.Serialize(textWriter, products.ToArray());
                    return textWriter.ToString();
                }
            });

            Get("/{name}", parameters =>
            {
                XmlSerializer xmlSerializer = new XmlSerializer(typeof(Product));
                
                for(int i = 0; i < products.Count; i++)
                {
                    if(parameters.name.ToString().Equals(products[i].Name.ToString())) {
                        using(StringWriter textWriter = new StringWriter()) {
                            xmlSerializer.Serialize(textWriter, products[i]);
                            return textWriter.ToString();
                        }
                    }
                }

                return "<Error>Not Found</Error>";
            });

            Post("/", parameters =>
            {
                XmlSerializer xmlSerializer = new XmlSerializer(typeof(Product));
                string payload = ((RequestStream) this.Request.Body).AsString();

                using (TextReader reader = new StringReader(payload))
                {
                    try {
                        Product p = (Product) xmlSerializer.Deserialize(reader);

                        if(products.Count < 10) {
                            products.Add(p);
                            return "<Error>Success</Error>";
                        }
                        else {
                            return "<Error>Too many objects</Error>";
                        }
                    }
                    catch(Exception exception) {
                        return "<Error>" + exception.Message + "</Error>";
                    }
                }
            });
        }
    }

    public class MainController : NancyModule
    {
        public MainController() : base("/") {
            Get("/", _ =>
            {
                return "Welcome to REST API v1.1\nREST API Endpoint /products is available.";
            });
        }
    }

    class Program
    {
        static void Main(string[] args)
        {
            var urlHost = "127.0.0.1";
            var urlPort = Environment.GetEnvironmentVariable("PORT") ?? "9000";
            var url = "http://" + urlHost + ":" + urlPort;

            using (var host = new NancyHost(new Uri(url)))
            {
                host.Start();
                Console.WriteLine("Running on " + url);
                Console.ReadLine();
            }
        }
    }
}
