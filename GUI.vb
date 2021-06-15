Imports System.IO
Imports System.Text
Imports System.Globalization

Public Class GUI
    Private Sub GUI_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        Watch(True)
        Dim aprocess() As System.Diagnostics.Process = System.Diagnostics.Process.GetProcessesByName("main")
        For Each proc As System.Diagnostics.Process In aprocess
            proc.Kill()
        Next
        PictureBox2.Visible = False
    End Sub

    Private Sub GUI_Closing(ByVal sender As Object, ByVal e As System.ComponentModel.CancelEventArgs) Handles MyBase.Closing
        Watch(True)
        Dim aprocess() As System.Diagnostics.Process = System.Diagnostics.Process.GetProcessesByName("main")
        For Each proc As System.Diagnostics.Process In aprocess
            proc.Kill()
        Next
    End Sub

    Private Sub TableLayoutPanel2_Paint(sender As Object, e As PaintEventArgs)

    End Sub

    Private Sub TextBox1_TextChanged(sender As Object, e As EventArgs) Handles TextBox1.TextChanged

    End Sub

    Private Sub TableLayoutPanel1_Paint(sender As Object, e As PaintEventArgs) Handles TableLayoutPanel1.Paint

    End Sub

    Private Sub TextBox2_TextChanged(sender As Object, e As EventArgs) Handles TextBox3.TextChanged

    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Mesh1.Click
        Dim file1location As String
        Dim file1name As String
        OpenFileDialog1.Filter = "PLY File|*.ply"
        OpenFileDialog1.InitialDirectory = "C:\Users\20181933\Documents\Jaar_3\Kwartiel_4\BEP_Medical_Imaging\Segmentations"
        OpenFileDialog1.Title = "Open PLY File"
        OpenFileDialog1.ShowDialog()

        file1location = OpenFileDialog1.FileName
        file1name = OpenFileDialog1.SafeFileName

        Mesh1.Text = file1name

    End Sub

    Private Sub Button2_Click(sender As Object, e As EventArgs) Handles Mesh2.Click
        Dim file2location As String
        Dim file2name As String
        OpenFileDialog2.ShowHelp = True
        OpenFileDialog2.Filter = "PLY File|*.ply"
        OpenFileDialog2.InitialDirectory = "C:\Users\20181933\Documents\Jaar_3\Kwartiel_4\BEP_Medical_Imaging\Segmentations"
        OpenFileDialog2.Title = "Open PLY File"
        OpenFileDialog2.ShowDialog()

        file2location = OpenFileDialog2.FileName
        file2name = OpenFileDialog2.SafeFileName

        Mesh2.Text = file2name

    End Sub

    Private Sub Button5_Click(sender As Object, e As EventArgs) Handles Button5.Click
        Dim savefile1location As String
        FolderBrowserDialog1.RootFolder = Environment.SpecialFolder.Desktop
        FolderBrowserDialog1.SelectedPath = "D:\BEP_Results"
        FolderBrowserDialog1.Description = "Select a Folder"
        FolderBrowserDialog1.ShowDialog()

        savefile1location = Path.GetFileName(FolderBrowserDialog1.SelectedPath)
        Button5.Text = savefile1location

    End Sub

    Private Sub Label5_Click(sender As Object, e As EventArgs) Handles Label5.Click

    End Sub

    Private Sub Label1_Click(sender As Object, e As EventArgs) Handles Label1.Click

    End Sub

    Private Sub Button1_Click_1(sender As Object, e As EventArgs) Handles Button1.Click
        Dim aprocess() As System.Diagnostics.Process = System.Diagnostics.Process.GetProcessesByName("main")
        For Each proc As System.Diagnostics.Process In aprocess
            proc.Kill()
        Next
        Watch(True)
        RichTextBox1.Text = ""
        Dim filePath As String = String.Format("C:\Users\20181933\Documents\Jaar_3\Kwartiel_4\BEP_Medical_Imaging\MainProgram\build\Parameters.txt")
        Dim Newstring1 As String
        Dim NewString2 As String
        Dim NewString3 As String
        Newstring1 = OpenFileDialog1.FileName.Replace("\", "/")
        NewString2 = OpenFileDialog2.FileName.Replace("\", "/")
        Newstring3 = FolderBrowserDialog1.SelectedPath.Replace("\", "/")
        If File.Exists(filePath) Then
            File.Delete(filePath)
            Using writer As New StreamWriter(filePath, True)
                writer.WriteLine(Newstring1)
                writer.WriteLine(NewString2)
                writer.WriteLine(NewString3)
                writer.WriteLine(TextBox1.Text)
                writer.WriteLine(TextBox2.Text)
                writer.WriteLine(TextBox3.Text)
                writer.WriteLine(TextBox4.Text)
                writer.WriteLine(TextBox5.Text)
                writer.WriteLine(TextBox6.Text)
            End Using
        Else
            Using writer As New StreamWriter(filePath, True)
                writer.WriteLine(Newstring1)
                writer.WriteLine(NewString2)
                writer.WriteLine(NewString3)
                writer.WriteLine(TextBox1.Text)
                writer.WriteLine(TextBox2.Text)
                writer.WriteLine(TextBox3.Text)
                writer.WriteLine(TextBox4.Text)
                writer.WriteLine(TextBox5.Text)
                writer.WriteLine(TextBox6.Text)
            End Using

        End If
        System.Diagnostics.Process.Start("C:\Users\20181933\Documents\Jaar_3\Kwartiel_4\BEP_Medical_Imaging\MainProgram\build\Release\main.exe")
        PictureBox2.Visible = True
        Watch(False)
    End Sub

    Public fswlist As New List(Of FileSystemWatcher)

    Private Sub Watch(StopWatching As Boolean)
        Threading.Thread.Sleep(1)
        If Not StopWatching Then
            Dim watcher As New IO.FileSystemWatcher()
            watcher.Path = FolderBrowserDialog1.SelectedPath + "\Logs"
            watcher.NotifyFilter = NotifyFilters.LastWrite Or NotifyFilters.FileName
            watcher.Filter = "log.txt"

            AddHandler watcher.Changed, AddressOf OnChanged
            AddHandler watcher.Created, AddressOf OnCreation
            watcher.EnableRaisingEvents = True
            fswlist.Add(watcher)
        End If
        If StopWatching Then
            For Each fsw In fswlist
                RemoveHandler fsw.Changed, AddressOf OnChanged
                RemoveHandler fsw.Created, AddressOf OnCreation
                fsw.EnableRaisingEvents = False
                fsw.Dispose()
            Next
            fswlist.Clear()
        End If

    End Sub

    Private Delegate Sub AppendTextBoxDelegate(ByVal TB As RichTextBox, ByVal txt As String)

    Private Sub AppendTextBox(ByVal TB As RichTextBox, ByVal txt As String)
        If TB.InvokeRequired Then
            TB.Invoke(New AppendTextBoxDelegate(AddressOf AppendTextBox), New Object() {TB, txt})
        Else
            TB.AppendText(txt)
        End If
    End Sub

    Private Sub OnChanged(sender As Object, e As FileSystemEventArgs)
        RichTextBox1.BeginInvoke(Sub() RichTextBox1.Clear())
        Dim csv As New FileStream(FolderBrowserDialog1.SelectedPath + "\Logs\log.txt", FileMode.Open, FileAccess.Read, FileShare.ReadWrite)
        Dim sr As New StreamReader(csv)
        While Not sr.EndOfStream
            AppendTextBox(RichTextBox1, sr.ReadLine())
            RichTextBox1.BeginInvoke(Sub() RichTextBox1.AppendText(vbNewLine))
        End While
        RichTextBox1.BeginInvoke(Sub() RichTextBox1.SelectionStart = RichTextBox1.Text.Length)
        RichTextBox1.BeginInvoke(Sub() RichTextBox1.ScrollToCaret())
        csv.Close()
        sr.Close()
    End Sub

    Private Sub OnCreation(sender As Object, e As FileSystemEventArgs)
        RichTextBox1.BeginInvoke(Sub() RichTextBox1.Clear())
        Dim csv As New FileStream(FolderBrowserDialog1.SelectedPath + "\Logs\log.txt", FileMode.Open, FileAccess.Read, FileShare.ReadWrite)
        Dim sr As New StreamReader(csv)
        While Not sr.EndOfStream
            AppendTextBox(RichTextBox1, sr.ReadLine())
            RichTextBox1.BeginInvoke(Sub() RichTextBox1.AppendText(vbNewLine))
        End While
        RichTextBox1.BeginInvoke(Sub() RichTextBox1.SelectionStart = RichTextBox1.Text.Length)
        RichTextBox1.BeginInvoke(Sub() RichTextBox1.ScrollToCaret())
        csv.Close()
        sr.Close()
    End Sub

    Private Sub Button2_Click_1(sender As Object, e As EventArgs) Handles Button2.Click
        PictureBox2.Visible = False
        Watch(True)
        Dim aprocess() As System.Diagnostics.Process = System.Diagnostics.Process.GetProcessesByName("main")
        For Each proc As System.Diagnostics.Process In aprocess
            proc.Kill()
        Next
        Dim JDATE As DateTime = DateTime.Now()
        Dim JDATE1 As String = JDATE.ToString("yyyy-MM-dd HH:mm:ss", System.Globalization.CultureInfo.InvariantCulture)
        RichTextBox1.AppendText(JDATE1 + vbTab + "Terminated Algorithm!" + vbNewLine)
    End Sub

    Private Sub TableLayoutPanel2_Paint_1(sender As Object, e As PaintEventArgs) Handles TableLayoutPanel2.Paint

    End Sub

    Private Sub RichTextBox1_TextChanged(sender As Object, e As EventArgs)

    End Sub

    Private Sub Button4_Click(sender As Object, e As EventArgs) Handles Button4.Click
        System.Diagnostics.Process.Start("C:\Program Files\ParaView 5.9.0-Windows-Python3.8-msvc2017-64bit\bin\paraview.exe")
    End Sub

    Private Sub TableLayoutPanel3_Paint(sender As Object, e As PaintEventArgs) Handles TableLayoutPanel3.Paint

    End Sub

    Private Sub Button3_Click(sender As Object, e As EventArgs) Handles Button3.Click
        System.Diagnostics.Process.Start("C:\Users\20181933\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Anaconda3 (64-bit)\Spyder (Anaconda3).lnk")
    End Sub

    Private Sub RichTextBox1_TextChanged_1(sender As Object, e As EventArgs) Handles RichTextBox1.TextChanged

    End Sub

    Private Sub OpenFileDialog1_FileOk(sender As Object, e As System.ComponentModel.CancelEventArgs) Handles OpenFileDialog1.FileOk

    End Sub

    Private Sub Label6_Click(sender As Object, e As EventArgs) Handles Label6.Click

    End Sub

    Private Sub NumericUpDown1_ValueChanged(sender As Object, e As EventArgs)

    End Sub
End Class
