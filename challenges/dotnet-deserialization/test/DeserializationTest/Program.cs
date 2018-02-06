using System;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Runtime.Serialization.Formatters.Binary;
using System.Runtime.Serialization;
using System.Xml;
using System.Xml.Serialization;

/*
BinaryFormatter
https://msdn.microsoft.com/en-us/library/system.runtime.serialization.formatters.binary.binaryformatter(v=vs.110).aspx

*/

namespace DeserializationTest
{
    public class SampleEntity
    {
        public string name;
        public string address;
    }

    class Program
    {
        static void CreateXMLSerializedFile(string path, SampleEntity[] entities)
        {
            XmlRootAttribute root = new XmlRootAttribute("Entities");
            XmlSerializer serializer = new XmlSerializer(typeof(SampleEntity[]), root);
            TextWriter writer = new StreamWriter(path);

            serializer.Serialize(writer, entities);
            writer.Close();
        }

        static SampleEntity[] ReadXMLSerializedFile(string path)
        {
            SampleEntity[] entities;
            FileStream fs = new FileStream(path, FileMode.Open);
            XmlReader reader = XmlReader.Create(fs);

            XmlRootAttribute root = new XmlRootAttribute("Entities");
            XmlSerializer serializer = new XmlSerializer(typeof(SampleEntity[]), root);

            entities = (SampleEntity[]) serializer.Deserialize(reader);
            reader.Close();
            
            return entities;
        }

        static void CreateBinaryFormatterFile(string path, object obj)
        {
            FileStream fs = new FileStream(path, FileMode.Create);
            BinaryFormatter formatter = new BinaryFormatter();

            formatter.Serialize(fs, obj);
            fs.Close();
        }

        static Hashtable ReadBinaryFormattedFile(string path)
        {
            Hashtable h = new Hashtable();
            FileStream fs = new FileStream(path, FileMode.Open);

            BinaryFormatter formatter = new BinaryFormatter();
            h = (Hashtable) formatter.Deserialize(fs);
            fs.Close();

            return h;
        }

        static void Main(string[] args)
        {
            Hashtable addresses = new Hashtable();
            
            addresses.Add("Jeff", "123 Main Street, Redmond, WA 98052");
            addresses.Add("Fred", "987 Pine Road, Phila., PA 19116");
            addresses.Add("Mary", "PO Box 112233, Palo Alto, CA 94301");

            if(args.Length > 1) {
                string cmd = args[0];
                string file = args[1];

                if(cmd.Equals("bin")) {
                    Console.WriteLine("Running BinaryFormatter test");
        
                    //Console.WriteLine("Creating file: " + file);
                    //CreateBinaryFormatterFile(file, addresses);

                    addresses = ReadBinaryFormattedFile(file);
                    Console.WriteLine("Addr of Jeff: " + addresses["Jeff"].ToString());
                }
                else if(cmd.Equals("xml")) {
                    SampleEntity[] entities = {
                        new SampleEntity { name = "Jeff", address = "123 Main Street, Redmond, WA 98052"},
                        new SampleEntity { name = "Fred", address = "987 Pine Road, Phila., PA 19116"},
                        new SampleEntity { name = "Mary", address = "PO Box 112233, Palo Alto, CA 94301"}
                    };

                    Console.WriteLine("Running XML serializer test");

                    //Console.WriteLine("Creating file: " + file);
                    //CreateXMLSerializedFile(file, entities);

                    Console.WriteLine("Reading XML data file: " + file);
                    SampleEntity[] readEntities = ReadXMLSerializedFile(file);

                    if(readEntities.Length > 0)
                        Console.WriteLine("Address of Jeff: " + readEntities[0].address);
                    else
                        Console.WriteLine("No entity in array");
                }
            }
        }
    }
}
